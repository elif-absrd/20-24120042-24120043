from mininet.net import Mininet
from mininet.node import OVSSwitch, RemoteController, Host
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel
from mininet.topo import Topo

class MyTopo(Topo):
    def build(self):
        # Add switches
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')

        # Add hosts with specified IPs
        h1 = self.addHost('h1', ip='10.1.1.2/24')  # Private IP for NAT
        h2 = self.addHost('h2', ip='10.1.1.3/24')  # Private IP for NAT
        h3 = self.addHost('h3', ip='10.0.0.4/24')
        h4 = self.addHost('h4', ip='10.0.0.5/24')
        h5 = self.addHost('h5', ip='10.0.0.6/24')
        h6 = self.addHost('h6', ip='10.0.0.7/24')
        h7 = self.addHost('h7', ip='10.0.0.8/24')
        h8 = self.addHost('h8', ip='10.0.0.9/24')
        h9 = self.addHost('h9', ip='172.16.10.10/24')  # Public IP for NAT

        # Add host-to-host links (h1-h9, h2-h9) with 5ms latency
        self.addLink(h1, h9, cls=TCLink, delay='5ms')
        self.addLink(h2, h9, cls=TCLink, delay='5ms')

        # Add host-to-switch links (5ms latency each)
        self.addLink(h9, s1, cls=TCLink, delay='5ms')  # New h9-s1 link
        self.addLink(h3, s2, cls=TCLink, delay='5ms')
        self.addLink(h4, s2, cls=TCLink, delay='5ms')
        self.addLink(h5, s3, cls=TCLink, delay='5ms')
        self.addLink(h6, s3, cls=TCLink, delay='5ms')
        self.addLink(h7, s4, cls=TCLink, delay='5ms')
        self.addLink(h8, s4, cls=TCLink, delay='5ms')

        # Add switch-to-switch links (7ms latency each)
        self.addLink(s1, s2, cls=TCLink, delay='7ms')
        self.addLink(s2, s3, cls=TCLink, delay='7ms')
        self.addLink(s3, s4, cls=TCLink, delay='7ms')
        self.addLink(s4, s1, cls=TCLink, delay='7ms')
        self.addLink(s1, s3, cls=TCLink, delay='7ms')

def configure_nat(h9, h1, h2):
    # Enable IP forwarding on h9
    h9.cmd('sysctl -w net.ipv4.ip_forward=1')

    # Flush existing iptables rules
    h9.cmd('iptables -F')
    h9.cmd('iptables -t nat -F')

    # Configure NAT (MASQUERADE) for h1 and h2 private IPs
    h9.cmd('iptables -t nat -A POSTROUTING -s 10.1.1.0/24 -o h9-eth1 -j MASQUERADE')

    # Set default gateway for h1 and h2 to h9's private interface
    h1.cmd('ip route add default via 10.1.1.1')
    h2.cmd('ip route add default via 10.1.1.1')

    # Assign private IP to h9's internal interface (h9-eth0)
    h9.cmd('ip addr add 10.1.1.1/24 dev h9-eth0')

def run():
    topo = MyTopo()
    net = Mininet(topo=topo, switch=OVSSwitch, 
                  controller=RemoteController('c0', ip='127.0.0.1', port=6633), 
                  link=TCLink)
    net.start()

    # Get host objects
    h1 = net.get('h1')
    h2 = net.get('h2')
    h9 = net.get('h9')

    # Configure NAT on h9
    configure_nat(h9, h1, h2)

    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run()