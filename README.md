# Network Topology

This section details the setup and execution of a network topology using Mininet and Ryu on Kali Linux within a Virtual Machine (VM) to study network loops (Part A) and resolve them with STP (Part B).

## Environment Setup

- Operating System: Working on Kali Linux installed in a Virtual Machine (VM) for network simulation.

- Ryu Installation:

  ```bash
  sudo apt update
  ```

  Updates the package lists on Kali Linux to ensure the latest versions are available.

  ```bash
  sudo apt install python3-pip python3-venv -y
  ```

  Installs Python 3, pip, and virtual environment tools without prompting for confirmation.

  ```bash
  python3 -m venv ~/ryu_env_39
  ```

  Creates a Python 3.9 virtual environment named `ryu_env_39` in the home directory.

  ```bash
  source ~/ryu_env_39/bin/activate
  ```

  Activates the virtual environment to isolate Ryu dependencies.

  ```bash
  pip install ryu
  ```

  Installs the Ryu SDN framework within the virtual environment.

- Activate Ryu Environment:

  ```bash
  source ~/ryu_env_39/bin/activate
  ```

  Activates the Ryu virtual environment for running controllers.

## Part A: Analyzing Network Loops Without STP

- Clean Mininet:

  ```bash
  sudo mn -c
  ```

  Clears any residual Mininet processes to ensure a fresh start.

- Launch Topology:

  ```bash
  sudo python3 topology.py
  ```

  Starts a Mininet topology with 4 switches (`s1-s4`), 8 hosts (`h1-h8`), and links (5ms host-switch, 7ms switch-switch). IPs are assigned as `h1: 10.0.0.2/24` to `h8: 10.0.0.9/24`.

- Run Ryu Controller (No STP):

  ```bash
  ryu-manager ryu.app.simple_switch_13 --ofp-tcp-listen-port 6633
  ```

  Runs Ryu with a basic L2 switch controller on port 6633, which lacks loop prevention.

- Ping Tests:

  ```bash
  h3 ping -c 10 10.0.0.2
  h5 ping -c 10 10.0.0.8
  h8 ping -c 10 10.0.0.3
  ```

  Executes 10 pings from `h3` to `h1`, `h5` to `h7`, and `h8` to `h2`, repeated 3 times with 30s intervals. All pings failed with 100% packet loss due to broadcast storms caused by loops.

## Part B: Resolving Loops with STP

- Clean Mininet:

  ```bash
  sudo mn -c
  ```

  Resets Mininet to clear previous configurations.

- Launch Topology:

  ```bash
  sudo python3 topology.py
  ```

  Starts the same topology as Part A, keeping it unchanged.

- Run Ryu Controller (With STP):

  ```bash
  ryu-manager ryu.app.simple_switch_stp_13 --ofp-tcp-listen-port 6633
  ```

  Runs Ryu with an STP-enabled controller on port 6633 to detect and block redundant paths.

- Wait for STP Convergence:

  ```bash
  py time.sleep(60)
  ```

  Pauses for 60 seconds in Mininet to allow STP to stabilize the network.

- Ping Tests:

  ```bash
  h3 ping -c 10 10.0.0.2
  h5 ping -c 10 10.0.0.8
  h8 ping -c 10 10.0.0.3
  ```

  Repeats the same ping tests as Part A, 3 times with 30s intervals. All pings succeeded with 0% loss, showing average RTTs of ~41ms (`h3-h1`), ~58ms (`h5-h7`), and ~44ms (`h8-h2`), confirming STP resolved the loops.

## Summary

- Part A: Used `simple_switch_13` on Kali Linux VM to demonstrate loop-induced network failure. Ryu logs showed excessive packet flooding, and Mininet confirmed ping failures.

- Part B: Switched to `simple_switch_stp_13`, enabling STP to block loops (e.g., `s3 port=3`), as observed in Ryu logs. Mininet pings succeeded, proving the fix worked.

---
