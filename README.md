# Vocal Intonation Booster - ESP32 Platform Port

The Vocal Intonation Booster ported to the Espressif ESP32 Audio Development Framework.

## Roadmap

- [X] Custom Audio Element w/ Passthrough (VIB_AE)
- [X] Bluetooth Low-Energy GATT Server
- [X] BLE-to-Audio briging system
- [ ] Integrate NE with VIB_AE
- [ ] Calculate F0 on VIB_AP external to VIB_AE processing task


## Brief

```
codec_chip ---> i2s_stream_reader ---> naturalear_element ---> i2s_stream_writer ---> codec_chip
```
