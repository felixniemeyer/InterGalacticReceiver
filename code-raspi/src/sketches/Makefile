SHADER_SCRIPT		= ./shader-includes.sh
SKETCH_DIRS			= $(wildcard */)
SHADER_OUT			= $(addsuffix /shaders.h,$(SKETCH_DIRS))

.PHONY : shaders
shaders : $(SHADER_OUT)

%/shaders.h : %/shader_template.h $(SHADER_SCRIPT)
	$(SHADER_SCRIPT) $(@D)

# Add .glsl dependency tracking per sketch
define add_glsl_dependency
$(1)/shaders.h : $(wildcard $(1)/*.glsl $(1)/*.frag $(1)/*.vert)
endef
$(foreach d,$(SKETCH_DIRS),$(eval $(call add_glsl_dependency,$(d))))