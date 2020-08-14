package=expat
$(package)_version=2.2.9
$(package)_download_path=https://github.com/libexpat/libexpat/releases/download/R_2_2_9/
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=1ea6965b15c2106b6bbe883397271c80dfa0331cdf821b2c319591b55eadc0a4

define $(package)_set_vars
  $(package)_config_opts=--disable-shared --without-docbook --without-tests --without-examples
  $(package)_config_opts += --disable-dependency-tracking --without-xmlwf
  $(package)_config_opts += --enable-option-checking
  $(package)_config_opts_linux=--with-pic
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef

define $(package)_postprocess_cmds
  rm lib/*.la
endef
