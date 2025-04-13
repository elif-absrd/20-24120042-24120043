from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSController, OVSSwitch
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel, info
import time

def create_network():
    # Initialize the Mininet environment with OVSController, OVSSwitch and TCLink
    network = Mininet(controller=OVSController, switch=OVSSwitch, link=TCLink)

    # Add the controller
    controller = network.addController('c0')

    # Define switches (s1, s2, s3, s4)
    s1 = network.addSwitch('s1')
    s2 = network.addSwitch('s2')
    s3 = network.addSwitch('s3')
    s4 = network.addSwitch('s4')

    # Define hosts with specific IP addresses
    h1 = network.addHost('h1', ip='10.1.1.2/24')  # Private host
    h2 = network.addHost('h2', ip='10.1.1.3/24')  # Private host
    h3 = network.addHost('h3', ip='10.0.0.4/24')
    h4 = network.addHost('h4', ip='10.0.0.5/24')
    h5 = network.addHost('h5', ip='10.0.0.6/24')
    h6 = network.addHost('h6', ip='10.0.0.7/24')
    h7 = network.addHost('h7', ip='10.0.0.8/24')
    h8 = network.addHost('h8', ip='10.0.0.9/24')
    
    # NAT device (h9) with dual interfaces
    h9 = network.addHost('h9')

    # Connect switches in a mesh topology with delays of 7ms
    network.addLink(s1, s2, delay='7ms')
    network.addLink(s2, s3, delay='7ms')
    network.addLink(s3, s4, delay='7ms')
    network.addLink(s4, s1, delay='7ms')
    network.addLink(s1, s3, delay='7ms')

    # Attach hosts to switches with 5ms delays
    network.addLink(h3, s2, delay='5ms')
    network.addLink(h4, s2, delay='5ms')
    network.addLink(h5, s3, delay='5ms')
    network.addLink(h6, s3, delay='5ms')
    network.addLink(h7, s4, delay='5ms')
    network.addLink(h8, s4, delay='5ms')
    
    # Connect NAT device (h9) to s1
    network.addLink(h9, s1, delay='5ms')
    
    # Connect private hosts to NAT device
    network.addLink(h1, h9, delay='5ms')
    network.addLink(h2, h9, delay='5ms')

    # Start the network
    network.start()

    # Enable STP on all switches to fix loop issues
    for switch in [s1, s2, s3, s4]:
        info(f'*** Enabling STP on {switch.name}\n')
        switch.cmd(f'ovs-vsctl set bridge {switch.name} stp_enable=true')
    
    # Wait for STP to converge
    info('*** Waiting for STP to converge (15 seconds)...\n')
    time.sleep(15)
    
    # Configure NAT device (h9)
    info('*** Configuring NAT on h9\n')
    
    # Setup interfaces on h9
    h9.cmd('ifconfig h9-eth0 172.16.10.10/24 up')
    h9.cmd('ifconfig h9-eth1 10.1.1.1/24 up')
    h9.cmd('ifconfig h9-eth2 10.1.1.1/24 up')
    
    # Enable IP forwarding
    h9.cmd('sysctl -w net.ipv4.ip_forward=1')
    
    # Clear any existing iptables rules
    h9.cmd('iptables -F')
    h9.cmd('iptables -t nat -F')
    
    # Set up NAT (masquerading)
    h9.cmd('iptables -t nat -A POSTROUTING -s 10.1.1.0/24 -o h9-eth0 -j MASQUERADE')
    
    # Allow forwarding for established connections and new connections from private network
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth0 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth2 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth0 -j ACCEPT')
    
    # Add static ARP entries and routes on h9
    h9.cmd('arp -s 10.1.1.2 $(ip neighbor | grep "h9-eth1" | awk \'{print $5}\')')
    h9.cmd('arp -s 10.1.1.3 $(ip neighbor | grep "h9-eth2" | awk \'{print $5}\')')
    
    # Configure default routes for private hosts (h1, h2)
    h1.cmd('ip route add default via 10.1.1.1')
    h2.cmd('ip route add default via 10.1.1.1')
    
    # Configure routes on public hosts to reach private subnet
    for h in [h3, h4, h5, h6, h7, h8]:
        h.cmd(f'ip route add 10.1.1.0/24 via 172.16.10.10')
    
    # Display iptables and routing configuration for debugging
    info('*** NAT configuration:\n')
    info(h9.cmd('iptables -t nat -L -v'))
    info(h9.cmd('ip route'))
    
    info('*** Network setup complete, running connectivity tests:\n')
    
    # Test private-to-public connectivity
    info(h1.cmd('ping -c 1 172.16.10.10'))
    info(h1.cmd('ping -c 1 10.0.0.6'))  # h5
    info(h2.cmd('ping -c 1 10.0.0.4'))  # h3
    
    # Launch CLI for interactive testing and management
    CLI(network)
    
    # Stop the network when done
    network.stop()

if _name_ == '_main_':
    setLogLevel('info')
    create_network()
