# RTCore64 Driver Troubleshooting Guide

This guide provides steps to diagnose and fix issues with the RTCore64 driver integration.

## Common Issues and Solutions

### 1. "Test read failed" Error

When you see "Test read failed" in the comprehensive test, this typically means:

- The driver is connected but memory reads are failing
- This could be due to access restrictions or driver configuration issues

**Solutions:**
- Run the diagnostics tool: `run_rtcore64_diagnostic.bat`
- Make sure you're running as Administrator
- Try installing the service with the fixed script: `install_rtcore64_service_fixed.bat`

### 2. Driver Not Found

If you see "RTCore64 driver not found" or cannot connect to the driver:

**Solutions:**
- Ensure MSI Afterburner is installed
- Check if the driver file exists at `C:\Program Files (x86)\MSI Afterburner\RTCore64.sys`
- Install/start the service with `install_rtcore64_service_fixed.bat`
- Run `sc query RTCore64` to verify the service status

### 3. Access Denied

If you see "Access denied" errors:

**Solutions:**
- Run all scripts and applications as Administrator
- Check if another application is using the driver
- Restart your computer and try again
- Make sure the service is properly installed and running

### 4. Memory Read/Write Failures

If memory operations fail:

**Solutions:**
- Try different memory addresses (our improved code now tries multiple addresses)
- Check error logs for specific error codes and messages
- Ensure the process has the necessary permissions
- Try smaller read/write operations
- Check if the antivirus is blocking the operations

## Diagnostic Tools

We've created several tools to help diagnose and fix RTCore64 issues:

1. **RTCore64_Diagnostic.exe** - Comprehensive diagnostics for the RTCore64 driver
2. **install_rtcore64_service_fixed.bat** - Properly installs and configures the RTCore64 service
3. **RTCore64_Simple_Test.exe** - Basic test for RTCore64 functionality

## Advanced Troubleshooting

If you're still having issues:

1. Check Windows Event Viewer for driver or service errors
2. Try reinstalling MSI Afterburner
3. Check for system conflicts with other kernel drivers
4. Look for detailed error information in the RTCore64_Error.log file

## Driver Security Note

RTCore64.sys is a vulnerable driver from MSI Afterburner. It's designed for use in controlled environments for educational purposes. Use with caution.
