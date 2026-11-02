if(CONFIG_ZENITH_QSPI_DIAG)
  zephyr_library_named(zenith_qspi_diag)
  zephyr_library_sources(qspi_diag.c)
endif()
