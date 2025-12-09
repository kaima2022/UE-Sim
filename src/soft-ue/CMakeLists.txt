build_lib(
  LIBNAME soft-ue
  SOURCE_FILES
    helper/soft-ue-helper.cc
    model/ses/ses-manager.cc
    model/ses/operation-metadata.cc
    model/pds/pds-manager.cc
    model/pds/pds-header.cc
    model/pds/pds-statistics.cc
    model/pdc/pdc-base.cc
    model/pdc/ipdc.cc
    model/pdc/tpdc.cc
    model/pdc/rto-timer/rto-timer.cc
    model/network/soft-ue-net-device.cc
    model/network/soft-ue-channel.cc
  HEADER_FILES
    helper/soft-ue-helper.h
    model/common/transport-layer.h
    model/common/soft-ue-packet-tag.h
    model/ses/ses-manager.h
    model/ses/operation-metadata.h
    model/ses/msn-entry.h
    model/pds/pds-manager.h
    model/pds/pds-common.h
    model/pdc/pdc-base.h
    model/pdc/ipdc.h
    model/pdc/tpdc.h
    model/pdc/rto-timer/rto-timer.h
    model/network/soft-ue-net-device.h
    model/network/soft-ue-channel.h
  LIBRARIES_TO_LINK ${libnetwork} ${libapplications} ${libinternet} ${libtraffic-control} ${libmobility} ${libcore}
  )

# Add Soft-UE test executables
# Module tests - Test core Soft-UE modules
add_executable(ses-manager-test test/modules/ses-manager-test.cc)
target_link_libraries(ses-manager-test ${libsoft-ue})

add_executable(pds-manager-test test/modules/pds-manager-test.cc)
target_link_libraries(pds-manager-test ${libsoft-ue})

add_executable(pdc-management-test test/modules/pdc-management-test.cc)
target_link_libraries(pdc-management-test ${libsoft-ue})

# End-to-end tests - Test complete Soft-UE functionality
add_executable(basic-communication-test test/end-to-end/basic-communication-test.cc)
target_link_libraries(basic-communication-test ${libsoft-ue} ${libmobility})

add_executable(simple-performance-test test/end-to-end/simple-performance-test.cc)
target_link_libraries(simple-performance-test ${libsoft-ue})

add_executable(integration-test test/end-to-end/integration-test.cc)
target_link_libraries(integration-test ${libsoft-ue} ${libmobility})