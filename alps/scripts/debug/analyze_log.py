import re
import intervaltree 

class AllocMap:
    def __init__(self):
        self.t = intervaltree.IntervalTree()

    def insert_alloc(self, offset, size):
        if self.t.overlaps(offset, offset+size):
            for i in self.t.search(offset,offset+size):
                print "0x%x %d" % (i.begin, i.end-i.begin)
            raise Exception("Overlapping allocation: 0x%x %d" % (offset, size))
        self.t[offset:offset+size] = 1


def analyze_log(path):
    am = AllocMap()

    with open(path, 'r') as f:
        for l in f:
            r = re.compile(".*Allocated block ptr: \((.*), (.*)\) size: (.*)")
            m = r.match(l)
            if m:
                region_id = m.group(1)
                offset = m.group(2)
                size = m.group(3)

                am.insert_alloc(int(offset, 16), int(size))
    

analyze_log('/tmp/out')       
