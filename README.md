# Device Standby
## Introduction<a name="section11660541593"></a>
To improve device endurance and reduce device power consumption, the system will limit the use of resources by backend applications when the device enters a standby idle state. Developers can apply for inclusion in standby resource control for their applications based on their own situation, or temporarily not be controlled by standby resources.

**Image 1**  Device Standby system structure

![](figures/zh-en_image.png)

## Directory Structure<a name="section161941989596"></a>

```
/foundation/resourceschedule/device_standby
├── frameworks       # API interface
├── interfaces
│   ├── innerkits    # inside interface
│   └── kits         # outside interface
├── sa_profile       # component service configruation
├── services         # service implements
└── utils            # commone utils
└── plugins          # plugins（state detection、decision、transition、exection）
└── bundle.json      # component discription and build file
```