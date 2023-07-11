set(COMPONENT_SRCS
  "main.c"

  "vib_audio.c"
  "vib_audio_pipeline.c"
  "vib_audio_element.c"
  "vib_audio_params.c"

  "vib_memory.c"

  "vib_ble_gap.c"
  "vib_ble_gatt_server.c"
  "vib_ble_gatt_table.c"
  "vib_ble_util.c"

)
set(COMPONENT_ADD_INCLUDEDIRS .)

register_component()
