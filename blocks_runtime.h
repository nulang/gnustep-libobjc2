/**
 * Block descriptor flags.  
 */
enum block_flags
{
	/**
	 * The block descriptor contains copy and dispose helpers.
	 */
	BLOCK_HAS_COPY_DISPOSE = (1 << 25),
	/**
	 * The helpers have C++ code.
	 */
	BLOCK_HAS_CTOR         = (1 << 26),
	/**
	 * Block is stored in global memory and does not need to be copied.
	 */
	BLOCK_IS_GLOBAL        = (1 << 28),
	/**
	 * Block function uses a calling convention that returns a structure via a
	 * pointer passed in by the caller.
	 */
	BLOCK_USE_SRET         = (1 << 29),
	/**
	 * Block has an Objective-C type encoding.
	 */
	BLOCK_HAS_SIGNATURE    = (1 << 30),
	/**
	 * Mask for the reference count in byref structure's flags field.  The low
	 * 3 bytes are reserved for the reference count, the top byte for the
	 * flags.
	 */
	BLOCK_REFCOUNT_MASK    = 0x00ffffff
};

/**
 * Flags used in the final argument to _Block_object_assign() and
 * _Block_object_dispose().  These indicate the type of copy or dispose to
 * perform.
 */
enum 
{
	/**
	 * The value is of some id-like type, and should be copied as an
	 * Objective-C object: i.e. by sending -retain or via the GC assign
	 * functions in GC mode (not yet supported).
	 */
	BLOCK_FIELD_IS_OBJECT   =  3,
	/**
	 * The field is a block.  This must be copied by the block copy functions.
	 */
	BLOCK_FIELD_IS_BLOCK	=  7,
	/**
	 * The field is an indirect reference to a variable declared with the
	 * __block storage qualifier.
	 */
	BLOCK_FIELD_IS_BYREF	=  8,  // the on stack structure holding the __block variable
	
	BLOCK_FIELD_IS_WEAK	 = 16,  // declared __weak
	
	BLOCK_BYREF_CALLER	  = 128, // called from byref copy/dispose helpers
};
#define IS_SET(x, y) ((x & y) == y)

/**
 * Block descriptor that contains copy and dispose operations.
 */
struct block_descriptor_copydispose
{
	/**
	 * Reserved for future use.  Currently always 0.
	 */
	unsigned long int reserved;
	/** Size of the block. */
	unsigned long int size;
	/**
	 * Copy function, generated by the compiler to help copy the block if it
	 * contains nontrivial copy operations.
	 */
	void (*copy_helper)(void *dst, void *src);
	/**
	 * Dispose function, generated by the compiler to help copy the block if it
	 * contains nontrivial destructors.
	 */
	void (*dispose_helper)(void *src); 
	/**
	 * Objective-C type encoding of the block.
	 */
	const char *encoding;
};
/**
 * Block descriptor that does not contain copy and dispose helper functions.
 */
struct block_descriptor
{
	/**
	 * Reserved for future use, currently always 0.
	 */
	unsigned long int reserved;
	/** Size of the block. */
	unsigned long int size;
	/**
	 * Objective-C type encoding of the block.
	 */
	const char *encoding;
};

// Helper structure
struct block_literal
{
	/**
	 * Class pointer.  Always initialised to &_NSConcreteStackBlock for blocks
	 * that are created on the stack or &_NSConcreteGlobalBlock for blocks that
	 * are created in global storage.
	 */
	void *isa;
	/**
	 * Flags.  See the block_flags enumerated type for possible values.
	 */
	int flags;
	/**
	 * Reserved - always initialised to 0 by the compiler.  Used for the
	 * reference count in this implementation.
	 */
	int reserved;
	/**
	 * The function that implements the block.  The first argument is this
	 * structure, the subsequent arguments are the block's explicit parameters.
	 * If the BLOCK_USE_SRET flag is set, there is an additional hidden
	 * argument, which is a pointer to the space on the stack allocated to hold
	 * the return value.
	 */
	void (*invoke)(void *, ...);
	/**
	 * The block's descriptor.  This is either block_descriptor or
	 * block_descriptor_copydispose, depending on whether the
	 * BLOCK_HAS_COPY_DISPOSE flag is set.
	 */
	struct block_descriptor_copydispose *descriptor;
	/**
	 * Block variables are appended to this structure.
	 */
};

/**
 * Structure used for on-stack variables that are referenced by blocks.
 */
struct block_byref_obj
{
	/**
	 * Class pointer.  Currently unused and always NULL.  Could be used in the
	 * future to support introspection.
	 */
	void *isa;
	/**
	 * The pointer to the structure that contains the real version of the data.
	 * All accesses go via this pointer.  If an on-stack byref structure is
	 * copied to the heap, then its forwarding pointer should point to the heap
	 * version.  Otherwise it should point to itself.
	 */
	struct block_byref_obj *forwarding;
	/**
	 * Flags and reference count.
	 */
	int flags;   //refcount;
	/**
	 * Size of this structure.
	 */
	int size;
	/**
	 * Copy function.
	 */
	void (*byref_keep)(struct block_byref_obj *dst, const struct block_byref_obj *src);
	/**
	 * Dispose function.
	 */
	void (*byref_dispose)(struct block_byref_obj *);
	/**
	 * __block-qualified variables are copied here.
	 */
};

