pylibsensors
============

Python bindings for libsensors on Linux

.. code-block:: python
    #!/usr/bin/env python3

    import math
    import sys
    import time

    import pylibsensors

    chips = pylibsensors.chips("coretemp-*")
    for chip_name,chip in chips.items():
        for feature in chip.features:
            for subfeature in chip[feature]:
                print(f'{chip_name} / {feature} / {subfeature} = {chip[feature][subfeature].value()}')

    print()

    chips = pylibsensors.chips()
    for chip_name,chip in chips.items():
        for feature in chip.features:
            for subfeature in chip[feature]:
                print(f'{chip_name} / {feature} / {subfeature} = {chip[feature][subfeature].value()}')
    print()

    try:
        while True:
            temp = chips['coretemp-isa-0000']['Package id 0']['temp1_input']
            volt = chips['BAT0-acpi-0']['in0']['in0_input']
            amps = chips['BAT0-acpi-0']['curr1']['curr1_input']
            print(f'> {temp.value()} / {volt.value()} / {amps.value()}')
            time.sleep(15)
    except KeyboardInterrupt:
        pass
