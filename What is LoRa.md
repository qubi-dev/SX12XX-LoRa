## What is LoRa?

LoRa is a type of chirp spread spectrum transmission that has been implemented by Semtech for use in low cost Industrial Scientific and Medical (ISM) band radio devices. The SX127x devices became available in 2014\2015. LoRa is short for Long Range.

As a general guide a LoRa device will have up to 10 times (or maybe more) the range \distance of other typical radio modules, when using the same frequencies, data rates, antennas and transmit powers.

The first LoRa devices were for the UHF bands and modules for 169Mhz, 434Mhz, 868Mhz and 915Mhz are available to match the ISM band requirements of a particular global location.

LoRa devices are programmed over an SPI interface. The older SX127x range of devices are the ones in common use and are register centric, they are programmed by changing the contents of 128 seperate single byte registers. Newer LoRa devices such as SX126x (UHF) and SX128x (2.4Ghz) are programmed through a command interface, you send a command byte followed by a series of parameter bytes across the SPI bus.

LoRa devices are packet based, you load a first in first out (FIFO) buffer with bytes of data and tell the device to transmit that packet of data. The packets can be up to 256 bytes long and can have an integral cyclic redundancy check (CRC) automatically added. When a packet is received it is loaded into the FIFO from where the bytes of data can be read out.

There are now several software libraries that allow LoRa devices to be used with a range of Arduino devices. Note that a library specifically for the SX1276 and SX1278 devices will not correctly drive an SX1272.  


#### Comparison to FSK devices

With frequency shift keying (FSK) style transmitters and receivers such as the Hope RFM22B or RFM69 for reception to work the receiver needs to see a signal that is strong enough to be above the local radio frequency (RF) noise level. Noise levels in the ISM bands are a performance limiter and each device in use adds to the local RF noise level. The total noise a receiver sees is a combination of local RF noise and noise generated by the electronics inside the receiver.

A typical FSK receiver needs to see a signal that is at least 5dB above the receiver's noise level in order to decode it. In comparison LoRa technology allows for signals to be received that are as much as 20dB below noise level, thus the LoRa signal could be up to 25dB (5dB + 20dB) weaker than the FSK one and still be received correctly. If LoRa can decode a signal that is 25dB weaker, then the transmitter can be further away than in the FSK example.

So due in the main to the below noise level performance, LoRa devices are able to receive signals that are very much weaker and thus much further away than with previous technology ISM band modules.


#### Configuring a LoRa module

LoRa modules are straight forward to drive and flexible, the internal firmware takes care of internal adjustments and optimisations, although there is an extra optimisation flag to set for long range slow to transmit packets.

The parameters that need to be configured for LoRa are, Spreading Factor, Coding Rate, Bandwidth, frequency and transmit power. These parameters can be easily changed so that you can set-up for low data rate long range or high data rate shorter range.

The LoRa device itself is a single surface mount integrated circuit, these are built onto small, typically 15mm x 15mm, modules that contain the crystal (or oscillator)  and antenna matching components the modules need. Examples of LoRa modules are the Hope RFM98 and Dorji DRF1278F.

LoRa is a two way technology, the transmitter device is a receiver too, so if you can pick-up signals from your transmitter you can send it commands or exchange data with it also.


#### Bandwidth

The LoRa bandwidth is the amount of frequency spread the signal occupies. The bandwidths LoRa can be set to are;

7.8kHz, 10.4kHz, 15.6kHz, 20.8kHz, 31.25kHz, 41.7kHz, 62.5kHz, 125kHz, 250kHz, 500kHz.

A low bandwidth signal, such as 7.8kHz will give the best (longest distance) performance, but with the low cost components used in the Hope and Dorji LoRa modules a transmitter and receiver may not talk to each other at this bandwidth. The receiver has a frequency capture range, this is the percentage difference allowable in relation to the bandwidth that the transmitter and receiver can be apart in frequency in order for the receiver to pick up the transmitters signal. This percentage is typically 25% of the bandwidth.

Lets assume the bandwidth in use is 10kHz, the allowable variation is 25% or 2.5kHz and the transmitter is on 434.000Mhz. For the link to work the receiver needs to be on or between 433.9975Mhz and 434.0025Mhz. This is an exacting requirement for a low cost crystal, even if transmitter and receiver are at the same temperature.

Low bandwidths of 20.8kHz have been used for long range trackers and they do give better performance, but care is needed as devices used at this bandwidth may not talk to each other unless a frequency calibration factor is measured and applied. Once communication is established at low bandwidth a form of automatic frequency control can be used to adjust the receivers base frequency to keep it within the capture range.

A bandwidth of 62.5kHz has been found to be reliable and allows for differences in manufacturing tolerance and temperature between transmitter and receiver devices. Applications such as The Things Network (see later) use a bandwidth of 125kHz.

Newer LoRa devices such as the SX1262 are designed to use temperature compensated crystal oscillators (TCXO). The TCXO is very stable in frequency and can allow LoRa devices to be used at the lowest bandwidth (7.8kHz) even when there are significant temperature differences between transmitting and receiving LoRa devices.


#### Spreading factor

This controls the amount of signal processing gain. The spreading factor can be set between 6 and 12. A setting of 6 can only be used with fixed length packets, so is not often used in applications.

The spreading factor defines the below noise performance. SF6 gives a -7.5dB below noise performance and SF12 a -20dB below noise performance.

The larger spreading factors therefore give more range, as it allows the reception of weaker signals, the trade off is that higher spreading factor packets take longer to transmit.


#### Coding rate

Additional data can be added to the packet to allow for error correction. This can result in small signal gains at the limit of reception. The coding rate can be varied between 4:5 and 4:8, the higher 4:8 rate will result in longer packets.

#### How the LoRa settings affect range

Having established a practical LoRa bandwidth, say 62.5kHz, we can use this base to look at how the spreading factor affects range and the time to transmit a 20 byte payload.

The lower spreading factors will give the highest data rates and the shortest packets in time terms which in turn is more battery efficient. The lowest spreading factor we will consider is 7.

Assume we are using a bandwidth of 62.5kHz and a spreading factor of 7, that will give an equivalent data rate of 2734bps, according Semtech's 'LoRa Calculator' tool. The time taken to transmit our 20 bytes is 98mS. Assume that is our base and that in a typical urban area the limit of reception is 500m.

If we switch to spreading factor 12 then the data rate drops to 146bps and the transmit time goes up to 2.2 seconds so this is a slow packet. The benefit of the higher spreading factor is significant, we now have an approximate 14dB of extra signal gain. This is equivalent to 5 times further range, so our 500m now extends to 2.5km. The 'LoRa Calculator' tool provides the signal gain figures for different combinations of spreading factor and bandwidth.  

5 times longer distance may not seem a lot, but appreciate how much difference it can make to the transmit power required to cover the extra distance. If we were transmitting  at 10mW and that gave us the 500m range, to cover 5 times further range just by increasing transmit power would require a transmitter putting out 25 times the 10mW or 250mW.

#### Data rates

The fastest data rate a SX127x UHF LoRa device is capable of is 37500bps.  The lowest data rate is 18bps, but the bandwidth at this rate, 7.8kHz, would be difficult to use.
Low data rate optimisation

When receiving a packet of data the receiver locks onto the packet preamble in order to synchronise the decoding of the following data. For the duration of packet reception only a small frequency variation in the transmitted signal is permitted. If the transmitter frequency drifts by more than circa 1.5% of the LoRa bandwidth in use, then there can be packet reception errors.

Unfortunately there will be a small amount of frequency drift during transmission caused by the LoRa device in the transmitter heating as the device dissipates the power used by the transmitter. The heating causes a slight drift in the frequency of the quartz crystal oscillator. The higher the transmit power the more the heating and the more the frequency drift. In long range mode, where packets may take a second or more to send, then there is a greater heating effect.  

Similar problems could occur when the frequency difference between transmitter and receiver occurs due to Doppler shift which can happen in communications between fast moving vehicles for instance.
To work around these issues, for packets that are slow to transmit and which have a symbol time of 12mS or longer, an optimisation flag is set. If you were using a bandwidth of 62.5kHz the optimisation flag needs to be set at spreading factor 10 and above.

#### RSSI versus SNR

LoRa devices can provide both the received signal strength indication (RSSI) and signal to noise ratio  (SNR) of a received packet. Whilst you might be tempted to use RSSI as an indication of the strength of the received signal, for weak signals, i.e. those approaching failure, RSSI is a poor indicator.

In practice the SNR reported is a far more useful indicator of impending link failure. If the SNR limit for
the spreading factor you are using is -10dB then a received packet reporting as SNR -4dB, is within 6dB of failure. Simple to do distance measurements can show that  with 6dB (10-4) of signal link margin remaining, you can expect to receive a packet at approximately twice distance.

SNR Limits for Spreading factor.

SF7 -7.5dB
SF8 -10dB
SF9 -12.5dB
SF10 -15dB
SF11 -17.5dB
SF12 -20dB 


#### Phantom Packets

It is an artefact of a LoRa receiver that data and packets can appear as if from nowhere, they come from within the device itself. The rate of phantom packets depends on the bandwidth in use by the receiver and can be at the rate of around 5 to 10 phantom packets per hour. Care is needed in software libraries to ensure that these phantoms are identified and rejected.


#### 2.4Ghz LoRa

More recently 2.4Ghz LoRa modules have been available, based on the SX128x. Whilst they don't have the extreme range of the UHF modules, they do allow for higher data rates, up to 200kbps and no duty cycle restrictions,.  

The 2.4Ghz LoRa modules include a ranging function which allows a suitably configured pair of modules to measure the distance between them. The ranging works best in clear open areas and can be used to measure distances from circa 50m to beyond 40km. Recent endeavours by the author have shown that 2.4GHz LoRa can work at up to 89km LOS, and the ranging at 85km LOS. See the report **[here](https://github.com/LoRaTracker/SX1280_Testing)**.

#### How far does LoRa go?

LoRa devices are capable of extreme long range at low data rates. The current distance record for standard modules is 766km, a ground based receiver tracking a high altitude balloon. This distance was achieved using only basic omnidirectional antennas, at 868Mhz and only 25mW transmit power. See **[here](https://www.hackster.io/news/a-new-lorawan-distance-record-577c8bc11d7b)**.   Its interesting to note that at 434MHz and the full device power of 100mW (when permitted!), the distance covered could be up to 4 times greater than the 766km.  

At the time of writing, June 2019, there are small Earth orbiting satellite launches planned that will utilise LoRa, so perhaps the 766km will indeed be beaten.


#### LoRa and LoRaWAN are different

LoRa devices are used for simple point to point communications. LoRaWAN is an extra software based network communication layer added to basic LoRa that allows sensors or nodes to communicate with gateway devices in a structured manner. These gateways can forward data received from nodes across the Internet to applications and data collection services running on remote servers. Internet services such as Cayenne allow you to view data from sensors on-line.

The Things Network (TTN) is a community based free to use network of LoRaWAN gateways. If you want to have a sensor monitor temperatures, water levels, or track the location of assets, you can use the TTN to do this, assuming a gateway is within range.

There is a fair access policy for using TTN, your restricted to 30 seconds of transmit time per day and can send 10 messages per day to the node from the gateway. If your node is close to a gateway and your node is using spreading factor 7 then you could transmit up to 14,000 bytes of data per day. Its best to plan for worst case where your node is a long distance from the gateway and spreading factor 12 needs to be used, in this case you can send only 600 bytes per day. TTN is not for applications that require large amounts of data to be sent.

<br>

### Stuart Robinson
### December 2019