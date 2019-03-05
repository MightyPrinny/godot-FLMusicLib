#ifndef RID_H
#define RID_H

#include <gdnative/rid.h>

namespace godot {

class Object;

class RID {
	godot_rid _godot_rid;

public:
	RID();

	RID(Object *p);

	int32_t get_rid() const;

	inline bool is_valid() const {
		// is_valid() is not available in the C API...
		return *this != RID();
	}

	bool operator==(const RID &p_other) const;
	bool operator!=(const RID &p_other) const;
	bool operator<(const RID &p_other) const;
	bool operator>(const RID &p_other) const;
	bool operator<=(const RID &p_other) const;
	bool operator>=(const RID &p_other) const;
};

} // namespace godot

#endif // RID_H
