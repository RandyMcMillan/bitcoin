package=native_clang
$(package)_version=11.0.0
$(package)_download_path=https://github.com/llvm/llvm-project/releases/download/llvmorg-$($(package)_version)
$(package)_download_file=clang+llvm-$($(package)_version)-x86_64-linux-gnu-ubuntu-20.04.tar.xz
$(package)_file_name=clang-llvm-$($(package)_version)-x86_64-linux-gnu-ubuntu-20.04.tar.xz
$(package)_sha256_hash=829f5fb0ebda1d8716464394f97d5475d465ddc7bea2879c0601316b611ff6db

define $(package)_preprocess_cmds
  rm -f $($(package)_extract_dir)/lib/libc++abi.so*
endef

define $(package)_stage_cmds
  mkdir -p $($(package)_staging_prefix_dir)/lib/clang/$($(package)_version)/include && \
  mkdir -p $($(package)_staging_prefix_dir)/bin && \
  mkdir -p $($(package)_staging_prefix_dir)/include && \
  cp bin/clang $($(package)_staging_prefix_dir)/bin/ && \
  cp -P bin/clang++ $($(package)_staging_prefix_dir)/bin/ && \
  cp bin/dsymutil $($(package)_staging_prefix_dir)/bin/$(host)-dsymutil && \
  cp bin/llvm-config $($(package)_staging_prefix_dir)/bin/ && \
  cp lib/libLTO.so $($(package)_staging_prefix_dir)/lib/ && \
  cp -rf lib/clang/$($(package)_version)/include/* $($(package)_staging_prefix_dir)/lib/clang/$($(package)_version)/include/
endef

define $(package)_postprocess_cmds
  rmdir include
endef
