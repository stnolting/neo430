## Alternative Top Entities for the NEO430 Processor

This folder provides several *alternative* TOP ENTITIES of the NEO430 processor.


### Default Top Entity

The default top entity of the processor is rtl/core/neo430_top.vhd. That entity propagates ALL signals
to the outer world and features a Wishbone bus interface. The type of all entity ports is
**std_ulogic** and **std_ulogic_vector**.


### Test Setup Top Entity

If you want to have a quick setup (that is also used for the implementation tutorial in the
project's documentary) you can use the *rlt/top_templates/neo430_test.vhd* as top entity. This entity only propagates
a minimal set of signals to the outer world (8 GPIO output signals and the UART lines). The test
setup is intended to be some kind of "hello world" demo.


### Top Entity with Resolved Signals

If you need resolved port signals instead of the default's top unresolved signals, you can use the
*rtl/top_templates/neo430_top_std_logic.vhd* as top entity. This entity uses **std_logic** and
**std_logic_vector** as interface types.


### Top Entity with Avalon Memory Mapped Master

*-> still experimental <-*

If you want to use the Avalon bus protocol instead of the default Wishbone bus connectivity, you
can use the *rtl/top_templates/neo430_top_avm.vhd* as top entity. This unit provides the same ports as the default top
entity, but it implements an Avalon-compatible master interface instead of a Wishbone master interface.

From a software point of view, the Avalon bus interface is used by calling the default Wishbone transfer
functions, since the native Wishbone interface is internally transformed to Avalon by a simple
combinatorial bridging logic.

Note: This setup also uses **std_logic** and **std_logic_vector** as port signal types to be compatible with
Quartus QSYS.


### Top Entity with AXI4-Lite Memory Mapped Master

*-> still experimental <-*

The *rtl/top_templates/neo430_top_axi4lite.vhd* top entity converts the processor's Wishbone bus to an AXI4-lite master compatible
interface. From a software point of view, the AXI4-lite interface is used by calling the default Wishbone
transfer functions.

Note: This setup also uses **std_logic** and **std_logic_vector** as port signal types.
