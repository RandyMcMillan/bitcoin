package=libxkbcommon
$(package)_version=0.6.1
$(package)_download_path=http://xkbcommon.org/download
$(package)_file_name=$(package)-$($(package)_version).tar.xz
$(package)_sha256_hash=5b0887b080b42169096a61106661f8d35bae783f8b6c58f97ebcd3af83ea8760

define $(package)_set_vars
$(package)_config_opts= --disable-static --disable-dependency-tracking --enable-option-checking
$(package)_config_opts+= --disable-docs --without-doxygen --disable-x11 --disable-shared
$(package)_config_opts+= --with-pic
endef

define $(package)_preprocess_cmds
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
  rm -rf share/man share/doc lib/*.la
endef
