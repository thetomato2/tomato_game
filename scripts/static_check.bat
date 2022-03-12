pushd T:\src

@echo ------------------------------
@echo STATICS FOUND:
@echo ------------------------------
findstr -s -n -i -l "static" *

@echo ------------------------------
@echo GLOBALS FOUND:
@echo ------------------------------
findstr -s -n -i -l "local_persist" *
findstr -s -n -i -l "global_var" *

popd
