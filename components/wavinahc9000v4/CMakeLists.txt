esphome_component_register(
  COMPONENT_NAME wavinahc9000v4
  SRC wavin_ahc9000.cpp wavin_zones.cpp
  INCLUDE_DIRS .
  REQUIRES uart climate sensor switch wifi
)
