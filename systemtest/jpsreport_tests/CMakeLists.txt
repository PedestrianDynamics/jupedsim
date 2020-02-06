list(APPEND METHODS
  "A"
  "B"
  "C"
  "D"
  "I"
  "J")
foreach (METHOD ${METHODS})
  add_test(NAME 01_method_${METHOD} COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/method_${METHOD}/01_method_${METHOD}.py)

  set_tests_properties(
    01_method_${METHOD}
    PROPERTIES LABELS "CI:FAST;jpsreport")
endforeach ()
