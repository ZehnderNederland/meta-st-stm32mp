#@DESCRIPTION: STM32MP machine flashlayout configuration

inherit flashlayout-stm32mp

# Add specific dependencies to get all binaries generated before flashlayout files
FLASHLAYOUT_DEPEND_TASKS += "${@bb.utils.contains('EXTRA_IMAGEDEPENDS', 'virtual/trusted-firmware-a', 'virtual/trusted-firmware-a:do_deploy', '', d)}"
FLASHLAYOUT_DEPEND_TASKS += "${@bb.utils.contains('EXTRA_IMAGEDEPENDS', 'virtual/bootloader', 'virtual/bootloader:do_deploy', '', d)}"
FLASHLAYOUT_DEPEND_TASKS += "${@bb.utils.contains('EXTRA_IMAGEDEPENDS', 'virtual/optee-os', 'virtual/optee-os:do_deploy', '', d)}"

FLASHLAYOUT_SUFFIX = "${@bb.utils.contains('ENCRYPT_ENABLE', '1', '${ENCRYPT_SUFFIX}', '', d)}${@bb.utils.contains('SIGN_ENABLE', '1', '${SIGN_SUFFIX}', '', d)}.tsv"

# List the PARTITION_TYPE used for bootloader partition
FLASHLAYOUT_BINARY_TYPES += "Binary"
FLASHLAYOUT_BINARY_TYPES += "ENV"
FLASHLAYOUT_BINARY_TYPES += "FIP"
FLASHLAYOUT_BINARY_TYPES += "FWU_MDATA"

# -----------------------------------------------------------------------------
# For the stm32mp flashlayout files, configuration is done to get output file
# with following name scheme:
#   FlashLayout_<CONFIG_LABEL>[_<TYPE_LABEL>-<BOOTSCHEME_LABEL>].tsv
#
# The different labels are stored as list in different vars:
#   - <BOOTSCHEME_LABEL>    FLASHLAYOUT_BOOTSCHEME_LABELS
#   - <CONFIG_LABEL>        FLASHLAYOUT_CONFIG_LABELS
#   - <TYPE_LABEL>          FLASHLAYOUT_TYPE_LABELS
#
# The flashlayout file name is generated with following loop logic:
#   for bootscheme in FLASHLAYOUT_BOOTSCHEME_LABELS     (expanded according to ongoing OVERRIDES)
#       for config in FLASHLAYOUT_CONFIG_LABELS         (expanded according to ongoing $bootscheme and OVERRIDES)
#           for typelabel in FLASHLAYOUT_TYPE_LABELS    (expanded according to ongoing $bootscheme, $config and OVERRIDES)
#               if FLASHLAYOUT_BOOTSCHEME_LABELS items > 1 && FLASHLAYOUT_TYPE_LABELS items > 1
#                   flashlayout_name = FlashLayout_$config_$typelabel-$bootscheme.tsv
#               else
#                   flashlayout_name = FlashLayout_$config.tsv
#
# Each flashlayout file should contain lines that set the partition properties:
#   <Opt> <Id> <Name> <Type> <IP> <Offset> <Binary>
#
# The partition properties are set with 'FLASHLAYOUT_PARTITION_xxx' vars:
#   - <Opt>     FLASHLAYOUT_PARTITION_ENABLE
#   - <Id>      FLASHLAYOUT_PARTITION_ID
#   - <Name>    Item from FLASHLAYOUT_PARTITION_LABELS list
#   - <Type>    FLASHLAYOUT_PARTITION_TYPE with optional FLASHLAYOUT_PARTITION_COPY (in case greater than 1)
#   - <IP>      FLASHLAYOUT_PARTITION_DEVICE
#   - <Offset>  FLASHLAYOUT_PARTITION_OFFSET
#   - <Binary>  FLASHLAYOUT_PARTITION_BIN2LOAD
#
# For the '$flashlayout_name' flashlayout file, the partition lines are generated
# with following loop logic:
#   for partition in FLASHLAYOUT_PARTITION_LABELS       (expanded according to ongoing $bootscheme, $config and OVERRIDES)
#       OVERRIDES_PREV = OVERRIDES
#       OVERRIDES = $partition:OVERRIDES
#       Compute partition properties for $partiton      (expanded according to ongoing $bootscheme, $config and OVERRIDES)
#       Write new partition line to $flashlayout_name
#       OVERRIDES = OVERRIDES_PREV
#
# Modifying the OVERRIDES inside the loop allows dynamic configuration for each
# partition property. This leaves plenty of possibility and guarantee specific
# '$partition' override.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Define bootscheme labels
# -----------------------------------------------------------------------------
FLASHLAYOUT_BOOTSCHEME_LABELS += "${BOOTSCHEME_LABELS}"

# -----------------------------------------------------------------------------
# Define config labels
# -----------------------------------------------------------------------------
# Priority var assignment (where <OVERRIDES> are the usual override mechanism):
#   1) FLASHLAYOUT_CONFIG_LABELS_<BOOTSCHEME>_<OVERRIDES>
#   2) FLASHLAYOUT_CONFIG_LABELS_<BOOTSCHEME>
#   3) FLASHLAYOUT_CONFIG_LABELS_<OVERRIDES>
#   4) FLASHLAYOUT_CONFIG_LABELS
# -----------------------------------------------------------------------------
FLASHLAYOUT_CONFIG_LABELS += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'emmc', 'emmc', '', d)}"
FLASHLAYOUT_CONFIG_LABELS += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nand-4-256', 'nand-4-256', '', d)}"
FLASHLAYOUT_CONFIG_LABELS += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor-sdcard', 'nor-sdcard', '', d)}"
FLASHLAYOUT_CONFIG_LABELS += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'nor', 'nor', '', d)}"
FLASHLAYOUT_CONFIG_LABELS += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'sdcard', bb.utils.contains('MACHINE_FEATURES', 'efi', 'sdcard sdcard-EFI', 'sdcard', d), '', d)}"
FLASHLAYOUT_CONFIG_LABELS += "${@bb.utils.contains('BOOTDEVICE_LABELS', 'spinand-2-128', 'spinand-2-128', '', d)}"

# Set by default this variable to 0, and set to 1 only when we are using st-example-image-*
ST_EXAMPLE_IMAGE ??= "0"

# Remove NAND flashlayout when we are using st-example-image-* as rootfs too big for a NAND device size of 1 GBytes
FLASHLAYOUT_CONFIG_LABELS:remove = "${@bb.utils.contains('ST_EXAMPLE_IMAGE', '1', 'nand-4-256', '', d)}"
FLASHLAYOUT_CONFIG_LABELS:remove = "${@bb.utils.contains('ST_EXAMPLE_IMAGE', '1', 'spinand-2-128', '', d)}"

# -----------------------------------------------------------------------------
# Define label types for each config
# -----------------------------------------------------------------------------
# NOTE: We use devicetree to allow proper filtering on supported storage device
# -----------------------------------------------------------------------------
# Priority var assignment (where <OVERRIDES> are the usual override mechanism):
#   1) FLASHLAYOUT_TYPE_LABELS:<BOOTSCHEME>:<CONFIG>:<OVERRIDES>
#   2) FLASHLAYOUT_TYPE_LABELS:<BOOTSCHEME>:<CONFIG>
#   3) FLASHLAYOUT_TYPE_LABELS:<BOOTSCHEME>:<OVERRIDES>
#   4) FLASHLAYOUT_TYPE_LABELS:<BOOTSCHEME>
#   5) FLASHLAYOUT_TYPE_LABELS:<CONFIG>:<OVERRIDES>
#   6) FLASHLAYOUT_TYPE_LABELS:<CONFIG>
#   7) FLASHLAYOUT_TYPE_LABELS:<OVERRIDES>
#   8) FLASHLAYOUT_TYPE_LABELS
# -----------------------------------------------------------------------------
# EMMC
# Set flashlayout file generation to eval board (mother and daughter) only
FLASHLAYOUT_TYPE_LABELS:emmc = "${DEVICE_BOARD_ENABLE:EMMC}"
# NAND
# Set flashlayout file generation to eval board only
FLASHLAYOUT_TYPE_LABELS:nand-4-256 = "${DEVICE_BOARD_ENABLE:NAND}"
# NOR
# Set flashlayout file generation to eval board only
FLASHLAYOUT_TYPE_LABELS:nor = "${DEVICE_BOARD_ENABLE:NOR}"
FLASHLAYOUT_TYPE_LABELS:nor-sdcard = "${DEVICE_BOARD_ENABLE:NOR}"
# SDCARD
# Set flashlayout file generation for all boards
FLASHLAYOUT_TYPE_LABELS:sdcard = "${DEVICE_BOARD_ENABLE:SDCARD}"
FLASHLAYOUT_TYPE_LABELS:sdcard-EFI = "${DEVICE_BOARD_ENABLE:SDCARD}"
# SPINAND
# Set flashlayout file generation for all boards
FLASHLAYOUT_TYPE_LABELS:spinand-2-128 = "${DEVICE_BOARD_ENABLE:SPINAND}"

# -----------------------------------------------------------------------------
# Define partitions to consider for flashlayout file generation
# -----------------------------------------------------------------------------
# Priority var assignment (where <OVERRIDES> are the usual override mechanism):
#   1) FLASHLAYOUT_PARTITION_LABELS:<BOOTSCHEME>:<CONFIG>:<OVERRIDES>
#   2) FLASHLAYOUT_PARTITION_LABELS:<BOOTSCHEME>:<CONFIG>
#   3) FLASHLAYOUT_PARTITION_LABELS:<BOOTSCHEME>:<OVERRIDES>
#   4) FLASHLAYOUT_PARTITION_LABELS:<BOOTSCHEME>
#   5) FLASHLAYOUT_PARTITION_LABELS:<CONFIG>:<OVERRIDES>
#   6) FLASHLAYOUT_PARTITION_LABELS:<CONFIG>
#   7) FLASHLAYOUT_PARTITION_LABELS:<OVERRIDES>
#   8) FLASHLAYOUT_PARTITION_LABELS
# -----------------------------------------------------------------------------
FLASHLAYOUT_PROGRAMMER_SECTIONS ?= "${STM32MP_FSBL_PROGAMMER_NAME} ${STM32MP_SSBL_PROGAMMER_NAME}"

# FLASHLAYOUT_PARTITION_IMAGES is initalized through PARTITIONS_CONFIG within 'flashlayout-stm32mp' class
FLASHLAYOUT_PARTITION_IMAGES ?= ""

FLASHLAYOUT_PARTITION_LABELS:emmc = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'emmc').split()])} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'emmc').split()])} \
    "
FLASHLAYOUT_PARTITION_LABELS:nand-4-256 = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'nand-4-256').split()])} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'nand-4-256').split()])} \
    "
FLASHLAYOUT_PARTITION_LABELS:nor = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'nor').split()])} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'nor').split()])} \
    "
FLASHLAYOUT_PARTITION_LABELS:nor-sdcard = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'nor-sdcard').split()])} \
    empty \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'nor-sdcard').split()])} \
    "
FLASHLAYOUT_PARTITION_LABELS:sdcard  = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'sdcard').split()])} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'sdcard').split()])} \
    "
FLASHLAYOUT_PARTITION_LABELS:sdcard-EFI = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'sdcard-EFI').split()])} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'sdcard-EFI').split()])} \
    "
FLASHLAYOUT_PARTITION_LABELS:spinand-2-128 = "\
    ${FLASHLAYOUT_PROGRAMMER_SECTIONS} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_BOOTLOADER_CONFIG', 'spinand-2-128').split()])} \
    ${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'spinand-2-128').split()])} \
    "

# -----------------------------------------------------------------------------
# Partition properties configuration
# -----------------------------------------------------------------------------
# Priority var assignment (where <OVERRIDES> are the usual override mechanism
# with dynamic append of ongoing 'partition name' item):
#           1) FLASHLAYOUT_PARTITION_xxx:<BOOTSCHEME>:<CONFIG>:<OVERRIDES>
#           2) FLASHLAYOUT_PARTITION_xxx:<BOOTSCHEME>:<CONFIG>
#           3) FLASHLAYOUT_PARTITION_xxx:<BOOTSCHEME>:<OVERRIDES>
#           4) FLASHLAYOUT_PARTITION_xxx:<BOOTSCHEME>
#           5) FLASHLAYOUT_PARTITION_xxx:<CONFIG>:<OVERRIDES>
#           6) FLASHLAYOUT_PARTITION_xxx:<CONFIG>
#           7) FLASHLAYOUT_PARTITION_xxx:<OVERRIDES>
#           8) FLASHLAYOUT_PARTITION_xxx
#           9) Default 'FLASHLAYOUT_PARTITION_xxx' to 'none' when not defined
# -----------------------------------------------------------------------------
FLASHLAYOUT_PARTITION_ENABLE = "P"
FLASHLAYOUT_PARTITION_ENABLE:${STM32MP_FSBL_PROGAMMER_NAME} = "-"
FLASHLAYOUT_PARTITION_ENABLE:${STM32MP_SSBL_PROGAMMER_NAME} = "-"
FLASHLAYOUT_PARTITION_ENABLE:empty = "PE"
# Need to make sure to delete partition that contains U-Boot env before update (gpt partitions only)
FLASHLAYOUT_PARTITION_ENABLE:sdcard:${STM32MP_SSBL1_NAME} = "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '', 'PD', d)}"
FLASHLAYOUT_PARTITION_ENABLE:sdcard-EFI:${STM32MP_SSBL1_NAME} = "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '', 'PD', d)}"
FLASHLAYOUT_PARTITION_ENABLE:emmc:${STM32MP_SSBL1_NAME} = "${@bb.utils.contains('ENABLE_FLASHLAYOUT_CONFIG_FWUP', '1', '', 'PD', d)}"

# -----------------------------------------------------------------------------
# Partition ID
# -----------------------------------------------------------------------------
# For FSBL and SSBL binaries loaded in RAM to program the devices there are two
# reserved IDs on STM32CubeProgrammer side:
#   0x01 for FSBL
#   0x03 for SSBL
FLASHLAYOUT_PARTITION_ID:${STM32MP_FSBL_PROGAMMER_NAME} = "0x01"
FLASHLAYOUT_PARTITION_ID:${STM32MP_SSBL_PROGAMMER_NAME} = "0x03"

FLASHLAYOUT_PARTITION_TYPE = "Binary"
FLASHLAYOUT_PARTITION_TYPE:${STM32MP_SSBL_PROGAMMER_NAME} = "FIP"
FLASHLAYOUT_PARTITION_TYPE:ubifs = "System"

FLASHLAYOUT_PARTITION_COPY = "1"

FLASHLAYOUT_PARTITION_DEVICE:emmc           = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:EMMC}:default"
FLASHLAYOUT_PARTITION_DEVICE:nand-4-256     = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:NAND}:default"
FLASHLAYOUT_PARTITION_DEVICE:nor            = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:NOR}:default"
FLASHLAYOUT_PARTITION_DEVICE:nor-sdcard     = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:NOR}:default,${DEVICE:SDCARD}:${@' '.join(['%s' % l.split(',')[1] for l in d.getVarFlag('PARTITIONS_CONFIG', 'nor-sdcard').split()])}"
FLASHLAYOUT_PARTITION_DEVICE:sdcard         = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:SDCARD}:default"
FLASHLAYOUT_PARTITION_DEVICE:sdcard-EFI     = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:SDCARD}:default"
FLASHLAYOUT_PARTITION_DEVICE:spinand-2-128  = "none:${FLASHLAYOUT_PROGRAMMER_SECTIONS},${DEVICE:SPINAND}:default"

FLASHLAYOUT_PARTITION_OFFSET:${STM32MP_FSBL_PROGAMMER_NAME} = "0x0"
FLASHLAYOUT_PARTITION_OFFSET:${STM32MP_SSBL_PROGAMMER_NAME} = "0x0"
FLASHLAYOUT_PARTITION_OFFSET:emmc:${STM32MP_FSBL1_NAME} = "${DEVICE_START_OFFSET_BOOT0:EMMC}"
FLASHLAYOUT_PARTITION_OFFSET:emmc:${STM32MP_FSBL2_NAME} = "${DEVICE_START_OFFSET_BOOT1:EMMC}"

# Size defined in Kbytes
FLASHLAYOUT_PARTITION_SIZE:empty = "0"
# Override default partition size for rootfs to increase available free space
# 3GiB for emmc
FLASHLAYOUT_PARTITION_SIZE:emmc:${STM32MP_ROOTFS_LABEL} = "3145728"
# 4GiB for sdcard
FLASHLAYOUT_PARTITION_SIZE:nor-sdcard:${STM32MP_ROOTFS_LABEL} = "4194304"
FLASHLAYOUT_PARTITION_SIZE:sdcard:${STM32MP_ROOTFS_LABEL}     = "4194304"
FLASHLAYOUT_PARTITION_SIZE:sdcard-EFI:${STM32MP_ROOTFS_LABEL} = "4194304"

# Set binaries to use for each partition
# -----------------------------------------------------------------------------
# Use following pattern in binary name to expand to specific label config:
#       '<CONFIG>'     (to insert label from FLASHLAYOUT_CONFIG_LABELS - NB: substitution for all '-' in label by '_')
#       '<BOOTSCHEME>' (to insert label from FLASHLAYOUT_BOOTSCHEME_LABELS)
#       '<DEVICE>'     (to insert label  from FLASHLAYOUT_PARTITION_DEVICE)
#       '<TYPE>'       (to insert label  from FLASHLAYOUT_TYPE_LABELS)
# These patterns are processed to expand binary name for each config.
# -----------------------------------------------------------------------------
FLASHLAYOUT_PARTITION_BIN2LOAD:${STM32MP_FSBL_PROGAMMER_NAME} = "arm-trusted-firmware/tf-a-<TYPE>-usb${TF_A_ENCRYPT_SUFFIX}${TF_A_SIGN_SUFFIX}.stm32"
FLASHLAYOUT_PARTITION_BIN2LOAD:${STM32MP_SSBL_PROGAMMER_NAME} = "${STM32MP_SSBL1_DATA}"

# -----------------------------------------------------------------------------
# Use the 'FLASHLAYOUT_PARTITION_REPLACE_PATTERNS' var to allow dynamic binary
# renaming for the bootloader binaries. This is only required for all FLASHLAYOUT_PROGRAMMER_SECTIONS
# partitions that provides the binary to flash the device.
# The format to follow is:
#   '<PATTERN2REPLACE_1>;<PATTERN2SET_1> <PATTERN2REPLACE_2>;<PATTERN2SET_2>'
# And the pattern to replace in binary name is searched as:
#   '[-_]<PATTERN>([-_.]|$)'
# -----------------------------------------------------------------------------
# The daughter board does not support Programmer mode, so use eval one
# (valid for FLASHLAYOUT_PROGRAMMER_SECTIONS partitions)
FLASHLAYOUT_PARTITION_REPLACE_PATTERNS:${STM32MP_FSBL_PROGAMMER_NAME}:append = " ed1;ev1"
FLASHLAYOUT_PARTITION_REPLACE_PATTERNS:${STM32MP_SSBL_PROGAMMER_NAME}:append = " ed1;ev1"
