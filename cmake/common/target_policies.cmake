# Shared target policy helpers for targets that compile JUCE module sources.
#
# Adapted from the atkAudio Plugin for OBS (AGPLv3).

include_guard(GLOBAL)

# JUCE modules are INTERFACE targets, so their known upstream diagnostics have to be
# suppressed on each concrete target that compiles the module implementation units.
function(echo_suppress_juce_target_warnings target_name)
  target_compile_options(
    ${target_name}
    PRIVATE
      $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-range-loop-bind-reference>
      $<$<COMPILE_LANG_AND_ID:C,Clang,AppleClang>:-Wno-ambiguous-macro>
      $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang>:-Wno-ambiguous-macro>
      $<$<COMPILE_LANG_AND_ID:OBJC,Clang,AppleClang>:-Wno-arc-repeated-use-of-weak>
      $<$<COMPILE_LANG_AND_ID:OBJCXX,Clang,AppleClang>:-Wno-arc-repeated-use-of-weak>
  )
endfunction()

function(echo_apply_msvc_juce_warning_overrides target_name)
  if(NOT MSVC)
    return()
  endif()

  # This target does not use C++ modules. Disabling MSVC's module scan avoids spurious
  # `*.module.json` warnings when multiple sources share a basename.
  set_property(TARGET ${target_name} PROPERTY CXX_SCAN_FOR_MODULES OFF)

  target_compile_options(${target_name} PRIVATE /wd4244 /wd4267 /wd4390 /wd5105)
endfunction()

function(echo_apply_juce_recommended_flags target_name)
  if(NOT TARGET ${target_name})
    return()
  endif()

  if(TARGET juce::juce_recommended_config_flags)
    target_link_libraries(${target_name} PRIVATE juce::juce_recommended_config_flags)
  endif()

  # LTO is intentionally CI-only to keep local iteration fast.
  if(TARGET juce::juce_recommended_lto_flags)
    target_link_libraries(
      ${target_name}
      PRIVATE $<$<OR:$<BOOL:$ENV{CI}>,$<BOOL:$ENV{GITHUB_ACTIONS}>>:juce::juce_recommended_lto_flags>
    )
  endif()
endfunction()
