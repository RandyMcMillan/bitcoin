package=musl_cross_make
$(package)_version=b12ded507831d0cac2dabd869aef14f3822a8770
$(package)_download_path=https://github.com/fanquake/musl-cross-make/archive
$(package)_download_file=$($(package)_version).tar.gz
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=21fc8d4d5ac241e57c73dfb1e4cdd8b6dcfb32db1276aa71e89be498a39e1980
$(package)_patches=config.mak

define $(package)_preprocess_cmds
  cp -f $($(package)_patch_dir)/config.mak config.mak && \
  sed -i.old "s|OUTPUT =|OUTPUT = $(build_prefix)|" config.mak
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) install
endef
