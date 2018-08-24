package=libpng
$(package)_version=1.6.37
$(package)_download_path=https://downloads.sourceforge.net/project/libpng/libpng16/$($(package)_version)/
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=c509d15ebdbfa355469828df2edcba15c5656761dd3037fcf28c206b5268a035
$(package)_dependencies=zlib

define $(package)_set_vars
  $(package)_config_opts=--enable-static --disable-shared
  $(package)_config_opts += --disable-dependency-tracking
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE) $($(package)_build_opts) PNG_COPTS='-fPIC'
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install $($(package)_build_opts)
endef

