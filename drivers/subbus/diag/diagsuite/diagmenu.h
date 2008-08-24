/* include the diagnostics you want in this file */

/* list of implemented tests to choose from */
/*
-------------------------------------------------------------------------
  "SUBBUS Low Byte:", "sbl",  subbus_low,
  "SUBBUS High Byte:", "sbh",  subbus_high,
  "SUBBUS Word:", "sbw",  subbus_word,
  "Non-Volatile RAM:", "pch",  nv_ram_test,
  "RAM Pattern Test:", "pat",  pattern_test,
  "CMDENBL Test:", "cmd",  cmdenbl_test,
  "Reboot Circuitry:", "rbt",  reboot_test,
  "Input Port:", "inp",  input_port,
  "NMI Test:", "nmi",  nmi_test
  "Addr Debug:", "adb", addr_debug,
  "Card AtoD0:", "ad0", AtoD0,
  "Card AtoD1:", "ad1", AtoD1,
  "Card AtoD2:", "ad2", AtoD2,
  "Card AtoD3:", "ad3", AtoD3,
  "Card AtoD4:", "ad4", AtoD4,
  "Card AtoD5:", "ad5", AtoD5,
  "Card AtoD6:", "ad6", AtoD6,
  "Card AtoD7:", "ad7", AtoD7,
  "Card H2O:",   "h2o", H2O,
  "DtoA Converter:", "d2a", DtoAtest
-----------------------------------------------------------------------
*/

/* menu name, 3 code mnemonic, name of diagnostic routine */
#ifdef card
  "Addr Debug:", "adb", addr_debug,
  "Card AtoD0:", "ad0", AtoD0,
  "Card AtoD1:", "ad1", AtoD1,
  "Card AtoD2:", "ad2", AtoD2,
  "Card AtoD3:", "ad3", AtoD3,
  "Card AtoD4:", "ad4", AtoD4,
  "Card AtoD5:", "ad5", AtoD5,
  "Card AtoD6:", "ad6", AtoD6,
  "Card AtoD7:", "ad7", AtoD7,
  "Card H2O:",   "h2o", H2O,
  "DtoA Converter:", "d2a", DtoAtest
#endif

#ifdef ana104
  "Addr Debug:", "adb", addr_debug,
  "AnI0 (17,18)->(95,96)", "ad0", AtoD0,
  "AnI1 (17,18)->(95,96)", "ad1", AtoD1,
  "AnI2 (17,18)->(95,96)", "ad2", AtoD2,
  "AnI3 (17,18)->(95,96)", "ad3", AtoD3,
  "AnI4 (17,18)->(95,96)", "ad4", AtoD4,
  "AnI5 (17,18)->(95,96)", "ad5", AtoD5,
  "AnI6 (17,18)->(95,96)", "ad6", AtoD6,
  "AnI7 (17,18)->(95,96)", "ad7", AtoD7,
  "AnO0 (1,2)->(15,16)]", "da0", DtoAtest0,
  "AnO1 (1,2)->(15,16)]", "da1", DtoAtest1,
  "AnO2 (1,2)->(15,16)]", "da2", DtoAtest2,
  "AnO3 (1,2)->(15,16)]", "da3", DtoAtest3,
  "AnO4 (1,2)->(15,16)]", "da4", DtoAtest4,
  "AnO5 (1,2)->(15,16)]", "da5", DtoAtest5,
  "AnO6 (1,2)->(15,16)]", "da6", DtoAtest6,
  "AnO7 (1,2)->(15,16)]", "da7", DtoAtest7
#endif

#ifdef syscon
  "SUBBUS Low Byte:", "sbl",  subbus_low,
  "SUBBUS High Byte:", "sbh",  subbus_high,
  "SUBBUS Word:", "sbw",  subbus_word,
  "Non-Volatile RAM:", "pch",  nv_ram_test,
  "RAM Pattern Test:", "pat",  pattern_test,
  "CMDENBL Test:", "cmd",  cmdenbl_test,
  "Reboot Circuitry:", "rbt",  reboot_test,
  "Input Port:", "inp",  input_port /* , */
/*  "NMI Test:", "nmi",  nmi_test */
#endif



