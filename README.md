# ESP32-C3 Web BLE Base Template

## 1. Prerequisites & Installation

To build this project, you need the **Espressif IoT Development Framework (ESP-IDF v6.1+)**.

***Easy installation:***
[Link to espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/get-started/index.html)

***Alternative***
[Link to esp-idf github](https://github.com/espressif/esp-idf)

### Step A: Install System Dependencies (macOS)


Install Dependencies:
cmake ninja python

# macOS 
Install Brew: 
[Link to brew](https://brew.sh)

```
brew install cmake ninja python
```

## Clone and install ESP-IDF:

```
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c3
```

## Export Environment (Required for every new terminal):

```
. $HOME/esp/esp-idf/export.sh
```

Tip: Add this line to your ~/.zshrc or ~/.bash_profile to make it permanent.

# 2. Building & Running
This project is configured to work "out of the box." You do not need to run menuconfig unless you want to change advanced settings.

## Step 1: Clone the Repo

```
git clone https://github.com/matias-villa-30/bella_balance.git
cd esp32-web-ble-base
```

## Step 2: Set Target & Build

Setting the target generates the local configuration based on our sdkconfig.defaults.

```
## Set hardware to ESP32-C3
idf.py set-target esp32c3
```


## Build, Flash, and Monitor in one command
## Replace [PORT] with your port (e.g., /dev/cu.usbmodem1101)

```
idf.py -p [PORT] build flash monitor
```

or 

```
idf.py build flash monitor
```


## Software: Run the Web App 
 ### 1. Open new terminal 
 ### 2. run the server from project dir

```
npx serve 
```

 ### 3. open provided url (usually localhost:3000) in Chrome browser

# 3. BLE Specifications
This template is pre-configured with the following GATT structure:
```
Element	UUID	Properties
Service	2d71a259-b458-c812-9999-4395122f4659	Primary
Characteristic	00000000-1111-1111-2222-222233333333	Write
Device Name: ESP32-SERVER
```

# 4. Project Structure
```
.
├── esp32
│   └── ble_server
│       ├── CMakeLists.txt
│       ├── dependencies.lock
│       ├── main
│       │   ├── ble_server.c
│       │   ├── CMakeLists.txt
│       │   └── idf_component.yml
│       ├── sdkconfig
│       ├── sdkconfig.defaults
│       └── sdkconfig.old
├── README.md
└── web
    └── index.html
```

sdkconfig.defaults: Crucial. Contains the pre-set Bluetooth configurations for the team.

# 5. Troubleshooting
Permission Denied (Serial): On Linux, run sudo usermod -a -G dialout $USER.

Unknown Component 'nimble': Ensure you have run the . export.sh command in your current terminal session.

Web BLE not finding device: Ensure you are using a Chromium-based browser (Chrome/Edge) and that your system Bluetooth is turned on.

Final Check for your Repo:

Make sure your ***.gitignore*** in the root folder contains exactly this to prevent team members from overwriting each other's local environment files:
```
Plaintext
build/
sdkconfig
sdkconfig.old
managed_components/
dependencies.lock
```
