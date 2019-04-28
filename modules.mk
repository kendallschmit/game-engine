# 1) name of program
define define_program
 out_file := $$(bindir)/$(1)

 $$(eval $$(call define_module,$(1),$$(out_file)))

 programs += $$(out_file)
endef

# 1) name of static library
define define_static_library
 out_file := $$(libdir)/lib$(1).a

 $$(eval $$(call define_module,$(1),$$(out_file)))

 static_libraries += $$(out_file)
 includedirs += $(1)/include
endef

# 1) name of module
# 2) file to output
define define_module
 # Convenient build rule
 $(1): $(2)

 # Find src + obj
 module_src := $$(wildcard $(1)/src/*)
 module_obj := $$(module_src:%.c=$$(builddir)/%.o)

 # Module output depends on objects and static libs
 $(2): $$(module_obj)

 # Add quoted include dir to CFLAGS
 $$(module_obj): CFLAGS += -iquote$(1)/include

 # Access to out_file
 module_out_file := $(2)

 # Track objects
 obj += $$(module_obj)
endef
