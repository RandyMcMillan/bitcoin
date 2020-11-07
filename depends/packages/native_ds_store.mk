package=native_ds_store
$(package)_version=67e238d3dc8427891d2c1c7f1d1fa6194c44a5a6
$(package)_download_path=https://github.com/al45tair/ds_store/archive/
$(package)_file_name=$($(package)_version).tar.gz
$(package)_sha256_hash=ba30d5126fd7ff50e6b1aa3d8a57ffb28c6000dd5ad0ca526d10d4fadb480a41
$(package)_install_libdir=$(build_prefix)/lib/python3/dist-packages

define $(package)_build_cmds
    python3 setup.py build
endef

define $(package)_stage_cmds
    mkdir -p $($(package)_install_libdir) && \
    python3 setup.py install --root=$($(package)_staging_dir) --prefix=$(build_prefix) --install-lib=$($(package)_install_libdir)
endef
