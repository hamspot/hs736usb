/**
 * HS-736USB Nano carrier — the Arduino Nano plugs into JP_L / JP_R.
 * Connector placement from the manual KiCad layout (origin 100,100; Y flipped).
 * CAT DIN pin 6 is NC. Power is Nano USB 5 V.
 * Connectivity is nets; copper is the autorouter.
 */
export default () => (
  <board
    title="HS-736USB Nano carrier"
    layers={2}
    autorouter="auto"
    outline={[
      { x: -25.6, y: 31 },
      { x: 25.6, y: 31 },
      { x: 25.6, y: -32.6 },
      { x: -25.6, y: -32.6 },
    ]}
  >
    <schematicsection name="Nano" displayName="Nano socket" />
    <schematicsection name="CAT" displayName="CAT jack" />
    <schematicsection name="Keying" displayName="PTT MOSFET + RCA" />
    <schematicsection name="LCD" displayName="HD44780 16x2" />
    <schematicsection name="IO" displayName="Jumper LED band PWM" />

    <net name="GND" isGroundNet />
    <net name="V5" isPowerNet />

    <hole diameter="3.2mm" pcbX={-16.8} pcbY={29.2} />
    <hole diameter="3.2mm" pcbX={23.8} pcbY={29.2} />
    <hole diameter="3.2mm" pcbX={-10} pcbY={-30.4} />
    <hole diameter="3.2mm" pcbX={10} pcbY={-30.4} />

    <silkscreentext text="USB" fontSize="1.2mm" pcbX={0} pcbY={29.6} />
    <silkscreentext text="HS-736USB" fontSize="1.4mm" pcbX={12} pcbY={29.6} />

    <pinheader
      name="JP_L"
      pinCount={15}
      gender="female"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={false}
      schSectionName="Nano"
      schX={-4}
      schY={4}
      schFacingDirection="left"
      schWidth={0.58}
      pcbX={-7.62}
      pcbY={6}
      pcbRotation={90}
      pinLabels={[
        "D13",
        "3V3",
        "AREF",
        "A0",
        "A1",
        "A2",
        "A3",
        "A4",
        "A5",
        "A6",
        "A7",
        "V5",
        "RST",
        "GND",
        "VIN",
      ]}
      pinAttributes={{
        "3V3": { doNotConnect: true },
        AREF: { doNotConnect: true },
        A2: { doNotConnect: true },
        A3: { doNotConnect: true },
        A6: { doNotConnect: true },
        A7: { doNotConnect: true },
        VIN: { doNotConnect: true },
      }}
      connections={{
        D13: "net.D13",
        A0: "net.A0",
        A1: "net.A1",
        A4: "net.SDA",
        A5: "net.SCL",
        V5: "net.V5",
        RST: "net.RST",
        GND: "net.GND",
      }}
    />
    <pinheader
      name="JP_R"
      pinCount={15}
      gender="female"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={false}
      schSectionName="Nano"
      schX={4}
      schY={4}
      schFacingDirection="right"
      schWidth={0.48}
      pcbX={7.62}
      pcbY={6}
      pcbRotation={90}
      pinLabels={[
        "D12",
        "D11",
        "D10",
        "D9",
        "D8",
        "D7",
        "D6",
        "D5",
        "D4",
        "D3",
        "D2",
        "GND",
        "RST",
        "RX0",
        "TX0",
      ]}
      pinAttributes={{
        D12: { doNotConnect: true },
        RX0: { doNotConnect: true },
        TX0: { doNotConnect: true },
      }}
      connections={{
        D11: "net.INTA",
        D10: "net.BUSY",
        D9: "net.SIN",
        D8: "net.SOUT",
        D7: "net.D7",
        D6: "net.D6",
        D5: "net.D5",
        D4: "net.D4",
        D3: "net.D3",
        D2: "net.D2",
        GND: "net.GND",
        RST: "net.RST",
      }}
    />
    <capacitor
      name="C1"
      capacitance="100nF"
      footprint="0805"
      schSectionName="Nano"
      schX={0}
      schY={1}
      schRotation={-90}
      pcbX={-12}
      pcbY={-6.7}
      pcbRotation={-90}
      connections={{ pin1: "net.V5", pin2: "net.GND" }}
    />

    <pinheader
      name="J_CAT"
      pinCount={6}
      gender="female"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="CAT"
      schX={14}
      schY={4}
      schWidth={0.58}
      pcbX={23}
      pcbY={17.19}
      pcbRotation={90}
      pinLabels={["GND", "SIN", "BUSY", "SOUT", "NC5", "NC6"]}
      pinAttributes={{
        NC5: { doNotConnect: true },
        NC6: { doNotConnect: true },
      }}
      connections={{
        GND: "net.GND",
        SIN: "net.SIN",
        BUSY: "net.BUSY",
        SOUT: "net.SOUT",
      }}
    />

    <resistor
      name="R_GATE"
      resistance="100ohm"
      footprint="0805"
      schSectionName="Keying"
      schX={24}
      schY={6}
      pcbX={11.2}
      pcbY={-1.62}
      connections={{ pin1: "net.D2", pin2: "net.QG" }}
    />
    <resistor
      name="R_GPD"
      resistance="10k"
      footprint="0805"
      schSectionName="Keying"
      schX={24}
      schY={4}
      schRotation={-90}
      pcbX={16}
      pcbY={-9.5}
      pcbRotation={90}
      connections={{ pin1: "net.QG", pin2: "net.GND" }}
    />
    <mosfet
      name="Q1"
      channelType="n"
      mosfetMode="enhancement"
      footprint="to92"
      schSectionName="Keying"
      schX={27}
      schY={5}
      pcbX={16}
      pcbY={-2}
      pcbRotation={180}
      connections={{
        gate: "net.QG",
        source: "net.GND",
        drain: "net.PTT_SW",
      }}
    />
    <diode
      name="D_FLY"
      footprint="sod123"
      schSectionName="Keying"
      schX={30}
      schY={6}
      pcbX={20}
      pcbY={-0.73}
      pcbRotation={90}
      connections={{ anode: "net.PTT_SW", cathode: "net.COIL12" }}
    />
    <pinheader
      name="J_AMP"
      pinCount={3}
      gender="male"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="Keying"
      schX={33}
      schY={4}
      schWidth={0.5}
      pcbX={23}
      pcbY={-2}
      pcbRotation={90}
      pinLabels={["COIL12", "DRAIN", "GND"]}
      connections={{
        COIL12: "net.COIL12",
        DRAIN: "net.PTT_SW",
        GND: "net.GND",
      }}
    />
    <resistor
      name="R_PTT"
      resistance="22k"
      footprint="0805"
      schSectionName="Keying"
      schX={24}
      schY={1}
      pcbX={16}
      pcbY={8.2}
      pcbRotation={180}
      connections={{ pin1: "net.RCA", pin2: "net.A0" }}
    />
    <diode
      name="D_Z"
      variant="zener"
      footprint="sod123"
      schSectionName="Keying"
      schX={27}
      schY={1}
      schRotation={180}
      pcbX={16}
      pcbY={3.8}
      pcbRotation={90}
      connections={{ cathode: "net.A0", anode: "net.GND" }}
    />
    <pinheader
      name="J_RCA"
      pinCount={2}
      gender="female"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="Keying"
      schX={30}
      schY={1}
      schWidth={0.5}
      pcbX={23}
      pcbY={5.77}
      pcbRotation={90}
      pinLabels={["TIP", "GND"]}
      connections={{ TIP: "net.RCA", GND: "net.GND" }}
    />

    <pinheader
      name="J_LCD"
      pinCount={16}
      gender="female"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="LCD"
      schX={0}
      schY={-8}
      schWidth={0.48}
      pcbX={-16.5}
      pcbY={4.89}
      pcbRotation={-90}
      pinLabels={[
        "VSS",
        "VDD",
        "VO",
        "RS",
        "RW",
        "E",
        "D0",
        "D1",
        "D2",
        "D3",
        "D4",
        "D5",
        "D6",
        "D7",
        "BLA",
        "BLK",
      ]}
      pinAttributes={{
        D0: { doNotConnect: true },
        D1: { doNotConnect: true },
        D2: { doNotConnect: true },
        D3: { doNotConnect: true },
      }}
      connections={{
        VSS: "net.GND",
        VDD: "net.V5",
        VO: "net.LCD_VO",
        RS: "net.LCD_RS",
        RW: "net.GND",
        E: "net.LCD_E",
        D4: "net.LCD_D4",
        D5: "net.LCD_D5",
        D6: "net.LCD_D6",
        D7: "net.LCD_D7",
        BLA: "net.V5",
        BLK: "net.GND",
      }}
    />
    <potentiometer
      name="RV1"
      maxResistance="10k"
      footprint="pinrow3"
      schSectionName="LCD"
      schX={6}
      schY={-8}
      pcbX={-21}
      pcbY={-20.5}
      pcbRotation={-90}
      connections={{ pin1: "net.V5", pin2: "net.LCD_VO", pin3: "net.GND" }}
    />

    <pinheader
      name="JP_A1"
      pinCount={2}
      gender="male"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="IO"
      schX={14}
      schY={-8}
      schWidth={0.5}
      pcbX={23}
      pcbY={-9.77}
      pcbRotation={90}
      pinLabels={["A1", "GND"]}
      connections={{ A1: "net.A1", GND: "net.GND" }}
    />
    <resistor
      name="R_LED"
      resistance="330ohm"
      footprint="0805"
      schSectionName="IO"
      schX={18}
      schY={-6}
      pcbX={-7.62}
      pcbY={27.5}
      pcbRotation={90}
      connections={{ pin1: "net.D13", pin2: "net.LED_A" }}
    />
    <led
      name="LED_DIAL"
      color="green"
      footprint="0805"
      schSectionName="IO"
      schX={23.1}
      schY={-6}
      schRotation={-90}
      pcbX={-12.2}
      pcbY={27.5}
      pcbRotation={180}
      connections={{ anode: "net.LED_A", cathode: "net.GND" }}
    />
    <pinheader
      name="J_BAND"
      pinCount={5}
      gender="male"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="IO"
      schX={18}
      schY={-10}
      schWidth={0.48}
      pcbX={23}
      pcbY={-20.04}
      pcbRotation={90}
      pinLabels={["D4", "D5", "D6", "D7", "GND"]}
      connections={{
        D4: "net.D4",
        D5: "net.D5",
        D6: "net.D6",
        D7: "net.D7",
        GND: "net.GND",
      }}
    />
    <pinheader
      name="J_PWM"
      pinCount={2}
      gender="male"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="IO"
      schX={22}
      schY={-10}
      schWidth={0.5}
      pcbX={-16.5}
      pcbY={-28.5}
      pcbRotation={-90}
      pinLabels={["D3", "GND"]}
      connections={{ D3: "net.D3", GND: "net.GND" }}
    />

    <chip
      name="U_MCP"
      footprint="soic28"
      manufacturerPartNumber="MCP23017"
      schSectionName="LCD"
      schX={10}
      schY={-8}
      pcbX={14}
      pcbY={-12}
      pinLabels={[
        "GPB0",
        "GPB1",
        "GPB2",
        "GPB3",
        "GPB4",
        "GPB5",
        "GPB6",
        "GPB7",
        "VDD",
        "VSS",
        "NC1",
        "SCL",
        "SDA",
        "NC2",
        "ADDR0",
        "ADDR1",
        "ADDR2",
        "RESET",
        "INTB",
        "INTA",
        "GPA0",
        "GPA1",
        "GPA2",
        "GPA3",
        "GPA4",
        "GPA5",
        "GPA6",
        "GPA7",
      ]}
      pinAttributes={{
        NC1: { doNotConnect: true },
        NC2: { doNotConnect: true },
        INTB: { doNotConnect: true },
        GPB3: { doNotConnect: true },
        GPB4: { doNotConnect: true },
        GPB5: { doNotConnect: true },
        GPB6: { doNotConnect: true },
        GPB7: { doNotConnect: true },
        GPA6: { doNotConnect: true },
        GPA7: { doNotConnect: true },
      }}
      connections={{
        GPB0: "net.ENC_A",
        GPB1: "net.ENC_B",
        GPB2: "net.ENC_SW",
        VDD: "net.V5",
        VSS: "net.GND",
        SCL: "net.SCL",
        SDA: "net.SDA",
        ADDR0: "net.GND",
        ADDR1: "net.GND",
        ADDR2: "net.GND",
        RESET: "net.V5",
        INTA: "net.INTA",
        GPA0: "net.LCD_D4",
        GPA1: "net.LCD_D5",
        GPA2: "net.LCD_D6",
        GPA3: "net.LCD_D7",
        GPA4: "net.LCD_RS",
        GPA5: "net.LCD_E",
      }}
    />
    <resistor
      name="R_SDA"
      resistance="4.7k"
      footprint="0805"
      schSectionName="LCD"
      schX={8}
      schY={-6}
      pcbX={8}
      pcbY={-12}
      connections={{ pin1: "net.SDA", pin2: "net.V5" }}
    />
    <resistor
      name="R_SCL"
      resistance="4.7k"
      footprint="0805"
      schSectionName="LCD"
      schX={8}
      schY={-5}
      pcbX={8}
      pcbY={-14}
      connections={{ pin1: "net.SCL", pin2: "net.V5" }}
    />
    <capacitor
      name="C_MCP"
      capacitance="100nF"
      footprint="0805"
      schSectionName="LCD"
      schX={12}
      schY={-6}
      pcbX={14}
      pcbY={-16}
      connections={{ pin1: "net.V5", pin2: "net.GND" }}
    />
    <pinheader
      name="J_ENC"
      pinCount={5}
      gender="male"
      pitch="2.54mm"
      showSilkscreenPinLabels={true}
      obstructsWithinBounds={true}
      schSectionName="IO"
      schX={24}
      schY={-8}
      schWidth={0.48}
      pcbX={-22}
      pcbY={-18}
      pcbRotation={-90}
      pinLabels={["V5", "GND", "A", "B", "SW"]}
      connections={{
        V5: "net.V5",
        GND: "net.GND",
        A: "net.ENC_A",
        B: "net.ENC_B",
        SW: "net.ENC_SW",
      }}
    />

  </board>
)
