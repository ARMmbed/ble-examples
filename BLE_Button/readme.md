To help you create your own BLE services, we've created this demo application.
The BLE_Button demonstrates the use of a simple input (boolean values) from a read-only characteristic.

The template covers:

1. Setting up advertising and connection states.

2. Assigning UUIDs to the service and its characteristic.

3. Creating an input characteristic: read-only, boolean, with notifications. This characteristic is updated according to the button's state.

4. Constructing a service class and adding it to the BLE stack.

Checking for Success
====================

Your Button peripheral should be detectable by BLE scanners (e.g. a smartphone)
and by the Google Physical Web app. You can connect to it and observe how the
read-only characteristic is automatically updated when you press the button
in your MCU (e.g NRF51-DK). The updates occur because your peripheral is pushing
notifications to the smartphone when it detects a change in the button.
