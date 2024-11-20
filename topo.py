from mininet.net import Mininet
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.node import OVSBridge
from mininet.term import makeTerms, makeTerm

class MyTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        s1 = self.addSwitch('s1')

        self.addLink(h1, s1, bw=100, delay='10ms')
        self.addLink(h2, s1)

topo = MyTopo()
net = Mininet(topo = topo, switch = OVSBridge, link = TCLink, controller = None)
net.start()
h1 = net.get('h1')
h2 = net.get('h2')
makeTerm(h1)
makeTerm(h2)
# h1.cmd('wireshark &')
# h1.cmd('make run -j')
# makeTerm(net.get('h1'), cmd="sudo ./http-server 2>&1 | tee h1_output.log")
# makeTerm(net.get('h2'), cmd="python3 test/mytest.py 2>&1 | tee h2_output.log")
CLI(net)
net.stop()