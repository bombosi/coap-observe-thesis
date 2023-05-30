# coap-observe-thesis
This is the code I have written for my thesis on exploring Iot protocols like MQTT and Coap, and the library i have written specifically 
to implement the observe mechanism in Coap protocol, using Coap-simple-library as a starting point and expanding it.

In the folder CoAP_simple_observe_library you can find a ready-to-use library based on coap-simple that implements the observe mechanism of CoAP,
following the specifics of RFC7641.

In the folder testcode_and_examples you can find other code I have written to experiment with ESP32 microcontroller and DHT sensor 
(a sensor to measure temperature and humidity), including a coap server that uses the library coap_simple_observe, a mqtt server, performance tests 
and a basic HTTP server.
