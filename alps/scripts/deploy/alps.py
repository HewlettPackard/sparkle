import argparse
import datetime
import logging
import os
import re
import sh
import shutil
import sys
import tempfile

import holodeck.context
import holodeck.mica.packaging as packaging
import holodeck.mica.repo_utils as repo_utils
import holodeck.common.config as config
import holodeck.mica.ansible as mica_ansible
from holodeck.common.exceptions import UsageError

import holodeck.mica.librarian as librarian_common

log = logging.getLogger(__name__)


DEFAULT_ARCHIVE_DIR = os.path.join(os.path.expanduser('~'), 'alps-jobs')


def extra_vars(cfg, librarian_ini):
    install_prefix = cfg.librarian.clients.install.prefix
    vl = [
        'lfs_node_count=%s' 
            % librarian_ini.node_count, 
        'lfs_book_size_bytes=%s' 
            % librarian_ini.book_size_bytes
    ]
    return vl


def package_alps():
    """
    Runs make package and returns the pathname to the created package or None
    if package creation failed.
    """
    build_dir = os.path.abspath('../../build')
    make = sh.Command("make").bake(_cwd = build_dir)
    out = make("package")

    rgx = re.compile("CPack: - package: (.*) generated")
    for l in out.split('\n'):
        m = rgx.match(l)
        if m:
            return str(m.group(1))
    return None
    

class InstallAction:
    @staticmethod
    def add_parser(subparsers):
        parser = subparsers.add_parser('install', help = "install Alps on client nodes")
        parser.set_defaults(func=InstallAction.action)

    @staticmethod
    def action(args):
        conf_file = args.conf_file
        cfg = config.YamlConfig(conf_file)

        librarian_common.verify_conf(cfg)
        librarian_ini = librarian_common.LibrarianIni(cfg.librarian.ini_file)

        ev = extra_vars(cfg, librarian_ini)
        inventory = mica_ansible.ClusterInventory(cfg.librarian.clients.cluster, "lfs_node={{ NR }}", ev)

        pkg_path = package_alps()

        # tasks that require privilege rights
        playbook = mica_ansible.PlayBook('alps-install-dep.yml', inventory, remote_user = 'root')
        playbook.add_task(mica_ansible.tasks.InstallMint())
        playbook.run()

        playbook = mica_ansible.PlayBook('alps-install-root.yml', inventory, remote_user = 'root')
        playbook.run()

        # regular tasks
        playbook = mica_ansible.PlayBook('alps-install.yml', inventory, remote_user = 'l4tm')
        playbook.add_task(mica_ansible.tasks.Unarchive("Alps", pkg_path, "/home/l4tm/testalps", 3))
        playbook.run()


def create_roles_inventory(cluster_name, user, dest_path):
    dest_stream = open(dest_path, 'w')
    mica_cluster_cmd = sh.Command(os.path.join(holodeck.context.holodeck_home_dir, 'bin/mica-cluster'))
    out = mica_cluster_cmd("-u", user, "hosts", cluster_name, "--format=mint")
    dest_stream.write(out.stdout)
    dest_stream.close()


def process_output(line):
    print line,


def run_test_suite(archive_dir, tests_regex, exclude_regex, *config_files):
    d = datetime.datetime.now()
    suite_archive_dir = os.path.join(archive_dir, "suite-%s" % d.strftime("%Y-%m-%d-%H%M%S"))

    mint_suite_cmd = sh.Command(os.path.join(holodeck.context.holodeck_home_dir, 'bin/mint-suite'))

    regex = ["--exclude-regex='.*tmpfs.*'"]
    if tests_regex:
        regex.append("--tests-regex=%s" % tests_regex)

    mint_suite_cmd("--archive=%s" % suite_archive_dir, regex,
                   config_files, _out=process_output, _err=process_output)

    print 'Archived results to:', suite_archive_dir


class UnitTestAction:
    @staticmethod
    def add_parser(subparsers):
        parser = subparsers.add_parser('unit', help = "run Alps unit tests on client nodes")
        parser.add_argument(
            "-a", "--archive_dir", dest='archive_dir', default=DEFAULT_ARCHIVE_DIR,
            help="directory where to archive test job results")
        parser.add_argument(
            "-r", "--tests_regex", dest='tests_regex', default=None,
            help="run tests with labels matching regular expression.")
        parser.set_defaults(func=UnitTestAction.action)

    @staticmethod
    def action(args):
        conf_file = args.conf_file
        cfg = config.YamlConfig(conf_file)

        librarian_common.verify_conf(cfg)

        try:
            tempdir_path = tempfile.mkdtemp()

            roles_path = os.path.join(tempdir_path, 'roles.yml')
            create_roles_inventory(cfg.librarian.clients.cluster, "l4tm", roles_path)

            # create test suite intentory for all unit tests
            mint_ctest_cmd = sh.Command(os.path.join(holodeck.context.holodeck_home_dir, 'bin/mint-ctest'))
            mint_ctest_cmd(os.path.join(cfg.alps.build_dir, "test/unit"), tempdir_path)

            # run test suite and archive results
            run_test_suite(args.archive_dir, args.tests_regex, "--exclude-regex='.*tmpfs.*'", 
                roles_path, 
                os.path.join(cfg.alps.home_dir, "test/unit/test_unit.yml"), 
                os.path.join(tempdir_path, "test_suite.yml"))
        finally:
            shutil.rmtree(tempdir_path)


class IntegrationTestAction:
    @staticmethod
    def add_parser(subparsers):
        parser = subparsers.add_parser('test', help = "run Alps integration tests on client nodes")
        parser.add_argument(
            "-a", "--archive_dir", dest='archive_dir', default=DEFAULT_ARCHIVE_DIR,
            help="directory where to archive test job results")
        parser.add_argument(
            "-r", "--tests_regex", dest='tests_regex', default=None,
            help="run tests with labels matching regular expression.")
        parser.set_defaults(func=IntegrationTestAction.action)

    @staticmethod
    def action(args):
        conf_file = args.conf_file
        cfg = config.YamlConfig(conf_file)

        librarian_common.verify_conf(cfg)

        try:
            tempdir_path = tempfile.mkdtemp()

            roles_path = os.path.join(tempdir_path, 'roles.yml')
            create_roles_inventory(cfg.librarian.clients.cluster, "l4tm", roles_path)

            # run test suite and archive results
            run_test_suite(args.archive_dir, args.tests_regex, None, 
                roles_path, 
                os.path.join(cfg.alps.home_dir, "test/integration/test_integration.yml"), 
                os.path.join(tempdir_path, "test/integration/test_suite.yml"))
        finally:
            shutil.rmtree(tempdir_path)
        

# Configure and parse our command-line arguments
def parse_args():
    parser = argparse.ArgumentParser(
                    prog = 'alps',
                    description='deploy Alps', 
                    formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    # parse parameters common to all commands
    parser.add_argument(
        "-c", "--conf_file", dest='conf_file', required=True,
        help="yaml configuration file descring Alps and Librarian configuration")
    parser.add_argument(
        "-v", "--verbose", dest='verbose', action='store_true',
        help="verbose")


    subparsers = parser.add_subparsers(dest='subparser_name', help='sub-command help')
    actions = [InstallAction, UnitTestAction, IntegrationTestAction]
    for a in actions:
        a.add_parser(subparsers)

    args = parser.parse_args()

    if args.verbose:
        logging.getLogger('').setLevel(logging.INFO)
    else:
        logging.getLogger('').setLevel(logging.ERROR)

    args.func(args)


def real_main():
    parse_args()


def main():
    try:
        real_main()
    except UsageError, e:
        print >> sys.stderr, "ERROR:", e
        sys.exit(1)


if __name__ == '__main__':
    main()
