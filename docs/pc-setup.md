# Preparing the PC for Wake-on-LAN

Wake-on-LAN fails silently, so set this up and test it before touching the cloud part.

## BIOS / UEFI

Names vary by vendor. The two settings that matter:

| What | ASUS | MSI | Gigabyte | ASRock |
|---|---|---|---|---|
| Disable deep power saving (otherwise the NIC loses power when the PC is off) | Advanced → APM → **ErP Ready = Disabled** | Power Management → **ErP Ready = Disabled** | Platform Power → **ErP = Disabled** | ACPI → **Deep Sleep = Disabled** |
| Enable wake from the network card | **Power On By PCI-E = Enabled** | **Resume By PCI-E Device = Enabled** | **Wake on LAN = Enabled** | **PCIE Devices Power On = Enabled** |

## Windows

1. **Fast Startup off**: Control Panel → Power Options → *Choose what the power buttons do* → uncheck *Turn on fast startup* (or `powercfg /h off` as admin).
2. **Network card** (Device Manager → Network adapters → your wired adapter → Properties):
   - *Power Management*: allow the device to wake the computer, **only a magic packet**.
   - *Advanced* (names vary; example for Intel I219-V):
     - Wake on Magic Packet: **Enabled**
     - Enable PME: **Enabled**
     - Wake on Pattern Match: **Disabled**
     - Wake on Link Settings: **Disabled**
     - Energy Efficient Ethernet: **Disabled**
     - Ultra Low Power Mode: **Disabled**
3. **MAC address**: `ipconfig /all` → *Physical Address* of the wired adapter.
4. **Allow ping** (used for the online/offline status), as admin:
   ```
   netsh advfirewall firewall add rule name="Allow Ping" protocol=icmpv4:8,any dir=in action=allow
   ```

## Router

Reserve a fixed IP for the PC and for the ESP32 (DHCP reservation by MAC address).
