# isogen/CMakeLists.txt

# Copyright (c) 2024 Omar Berrow

# The only point of this subdirectory is to provide a target which builds the iso.

set (SOURCE_DIR ${CMAKE_SOURCE_DIR})
if (CMAKE_HOST_WIN32)
	set (SUPPRESS_OUTPUT > NUL 2>&1)
elseif(CMAKE_HOST_UNIX)
	set (SUPPRESS_OUTPUT > /dev/null 2>&1)
endif()

set (ISODIR ${CMAKE_SOURCE_DIR}/isodir/)

if (NOT EXISTS ${ISODIR})
	file (MAKE_DIRECTORY ${ISODIR})
endif()
if (NOT EXISTS ${ISODIR}/obos)
	file (MAKE_DIRECTORY ${ISODIR}/obos)
endif()
if (NOT EXISTS ${ISODIR}/limine)
	file (MAKE_DIRECTORY ${ISODIR}/limine)
endif()
if (NOT EXISTS ${ISODIR}/EFI)
	file (MAKE_DIRECTORY ${ISODIR}/EFI)
endif()
if (NOT EXISTS ${ISODIR}/EFI/BOOT)
	file (MAKE_DIRECTORY ${ISODIR}/EFI/BOOT)
endif()
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/config/limine.cfg)
	message(FATAL_ERROR "No hyper configuration file detected!")
endif()
add_custom_target(isogen ALL
	COMMAND ${OBJCOPY} -g ${OUTPUT_DIR}/oboskrnl ${ISODIR}/obos/oboskrnl
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/config/limine.cfg ${ISODIR}/ ${SUPPRESS_OUTPUT}
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-uefi-cd.bin ${ISODIR}/limine ${SUPPRESS_OUTPUT}
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-bios-cd.bin ${ISODIR}/limine ${SUPPRESS_OUTPUT}
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-bios-pxe.bin ${ISODIR}/limine ${SUPPRESS_OUTPUT}
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-bios.sys ${ISODIR}/limine ${SUPPRESS_OUTPUT}
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dependencies/limine/BOOTX64.EFI ${ISODIR}/EFI/BOOT ${SUPPRESS_OUTPUT}
	COMMAND	${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/dependencies/limine/BOOTIA32.EFI ${ISODIR}/EFI/BOOT ${SUPPRESS_OUTPUT}
	COMMAND	xorriso -as mkisofs -b limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label isodir -o ${OUTPUT_DIR}/obos.iso ${SUPPRESS_OUTPUT}
	COMMAND	${limine_install} bios-install ${OUTPUT_DIR}/obos.iso ${SUPPRESS_OUTPUT}
	SOURCES ${CMAKE_SOURCE_DIR}/config/limine.cfg ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-uefi-cd.bin ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-bios-cd.bin
            ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-bios-pxe.bin ${CMAKE_SOURCE_DIR}/dependencies/limine/limine-bios.sys ${CMAKE_SOURCE_DIR}/dependencies/limine/BOOTX64.EFI
			${CMAKE_SOURCE_DIR}/dependencies/limine/BOOTIA32.EFI
	DEPENDS oboskrnl
	BYPRODUCTS ${ISODIR}/limine.cfg
	BYPRODUCTS ${ISODIR}/EFI/BOOT/BOOTX64.efi
	BYPRODUCTS ${ISODIR}/EFI/BOOT/BOOTIA32.efi
	BYPRODUCTS ${ISODIR}/limine/limine-uefi-cd.bin
	BYPRODUCTS ${ISODIR}/limine/limine-bios-cd.bin
	BYPRODUCTS ${ISODIR}/limine/limine-bios-pxe.bin
	BYPRODUCTS ${ISODIR}/limine/limine-bios.sys
	BYPRODUCTS ${ISODIR}/obos/oboskrnl
	BYPRODUCTS ${OUTPUT_DIR}/obos.iso
	# DEPENDS oboskrnl
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Generating ${OUTPUT_DIR}obos.iso"
)