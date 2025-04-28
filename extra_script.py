Import("env")

def get_upload_port():
    # Check for usbmodem port first (ESP32-S3 native USB)
    import glob
    usbmodem_ports = glob.glob("/dev/cu.usbmodem*")
    if usbmodem_ports:
        print(f"Found native USB port: {usbmodem_ports[0]}")
        return usbmodem_ports[0]
    
    # Fallback to CH340 port
    wch_ports = glob.glob("/dev/cu.wchusbserial*")
    if wch_ports:
        print(f"Found CH340 serial port: {wch_ports[0]}")
        return wch_ports[0]
    
    # No port found
    print("No suitable upload port found!")
    return None

port = get_upload_port()
if port:
    print(f"Setting upload port to: {port}")
    env.Replace(UPLOAD_PORT=port)
    env.Replace(MONITOR_PORT=port)