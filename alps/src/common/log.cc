/* 
 * (c) Copyright 2016 Hewlett Packard Enterprise Development LP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "log.hh"

#include <boost/log/core/core.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/utility/empty_deleter.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <ostream>

#include "alps/common/debug_options.hh"
 
namespace alps {

namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
 
BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", boost::log::trivial::severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type)
 
boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger;

void init_log(const DebugOptions& debug_options)
{
    // add attributes
    logger.add_attribute("LineID", attrs::counter<unsigned int>(1));     // lines are sequentially numbered
    logger.add_attribute("TimeStamp", attrs::local_clock());             // each log line gets a timestamp
    logger.add_attribute("ThreadID", boost::log::attributes::current_thread_id()); // each log line gets a thread_id

    // construct and add a text sink
    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

    // add a stream to write log to
    sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(debug_options.log_filename));

    // add "console" output stream to our sink
    // we have to provide an empty deleter to avoid destroying the global stream object
    boost::shared_ptr< std::ostream > stream(&std::clog, boost::empty_deleter());
    sink->locked_backend()->add_stream(stream);

    // specify the format of the log message
    boost::log::formatter formatter = expr::stream
      << std::setw(7) << std::setfill('0') << line_id << std::setfill(' ') << " | "
      << "T." << thread_id << " | " 
      << expr::format_date_time(timestamp, "%Y-%m-%d, %H:%M:%S.%f") << " "
      << "[" << boost::log::trivial::severity << "]"
      << " - " << expr::smessage;
    
    sink->set_formatter(formatter);

    std::string sl_str = debug_options.log_level;

    boost::log::trivial::severity_level sl;
    std::transform(sl_str.begin(), sl_str.end(), sl_str.begin(), ::tolower);
    if (sl_str == "trace") {
      sl = boost::log::trivial::severity_level::trace;
    } 
    else if (sl_str == "debug") {
      sl = boost::log::trivial::severity_level::debug;
    }
    else if (sl_str == "info") {
      sl = boost::log::trivial::severity_level::info;
    }
    else if (sl_str == "warning") {
      sl = boost::log::trivial::severity_level::warning;
    }
    else if (sl_str == "error") {
      sl = boost::log::trivial::severity_level::error;
    }
    else if (sl_str == "fatal") {
      sl = boost::log::trivial::severity_level::fatal;
    }
    else {
        // no matching string -- check numerical
        int il = std::stoi(sl_str);
        if (il >= boost::log::trivial::severity_level::trace and
            il <= boost::log::trivial::severity_level::fatal)
        {
            sl = static_cast<boost::log::trivial::severity_level>(il);
        }
    }
    sink->set_filter(severity >= sl);

    // register the sink in the logging core
    boost::log::core::get()->add_sink(sink);
}

} // namespace alps
