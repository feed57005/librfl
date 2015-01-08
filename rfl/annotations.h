#ifndef __LIBRFL_ANNOTATIONS_H__
#define __LIBRFL_ANNOTATIONS_H__

#if defined(__RFL_SCAN__)
#define rfl_class(...) __attribute__((annotate("class:" #__VA_ARGS__)))
#define rfl_enum(...) __attribute__((annotate("enum:" #__VA_ARGS__)))
#define rfl_property(...) __attribute__((annotate("property:" #__VA_ARGS__)))
#define rfl_field(...) __attribute__((annotate("field:" #__VA_ARGS__)))
#else
#define rfl_class(...)
#define rfl_enum(...)
#define rfl_property(...)
#define rfl_field(...)
#endif


#endif /* __LIBRFL_ANNOTATIONS_H__ */
