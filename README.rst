pylibsensors
============

Python bindings for libsensors on Linux

.. code-block:: python

   import time
   import pylibsensors
   
   chips = pylibsensors.chips()
   for chip_name,chip in chips.items():
       for feature in chip.features:
           for subfeature in chip[feature]:
               print(f'{chip_name} / {feature} / {subfeature} = {chip[feature][subfeature].value()}')
   print()
   
   temp = chips['coretemp-isa-0000']['Package id 0']['temp1_input']
   volt = chips['BAT0-acpi-0']['in0']['in0_input']
   amps = chips['BAT0-acpi-0']['curr1']['curr1_input']
   
   try:
       while True:
           print(f'> {temp.value()} / {volt.value()} / {amps.value()}')
           time.sleep(15)
   except KeyboardInterrupt:
       pass
