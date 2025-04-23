idf_component_register(
    SRCS 
        "src/stusb4500_core.cpp"
        "src/stusb4500_pdo.cpp"
        "src/stusb4500_nvm_decode.cpp"
        "src/stusb4500_config.cpp"
        "src/stusb4500_decode.cpp"
        "src/stusb4500_sync.cpp"
    INCLUDE_DIRS "include"
    REQUIRES driver I2CDevice
) 
