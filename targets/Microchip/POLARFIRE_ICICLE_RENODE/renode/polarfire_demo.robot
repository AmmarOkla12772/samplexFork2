*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Setup      Reset Emulation
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}

*** Test Cases ***
Should Boot ThreadX And Trigger Telemetry Alarms
    Execute Command          include @${CURDIR}/polarfire_demo.resc
    Create Terminal Tester   sysbus.mmuart1

    Wait For Line On Uart    Microchip PolarFire SoC Icicle Kit (Renode Target)             timeout=10
    Wait For Line On Uart    [Monitor] ThreadX Ticks: 0 | Memory Area Active | Queues OK     timeout=10
    Wait For Line On Uart    [Monitor] ThreadX Ticks: 100 | Memory Area Active | Queues OK    timeout=10
    Wait For Line On Uart    OVERTEMP ALARM TRIGGERED                                         timeout=15
