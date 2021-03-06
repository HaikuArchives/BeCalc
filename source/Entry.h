// Entry.h

struct entry_ref {
	entry_ref();
	entry_ref(dev_t device, ino_t dir, const char *name);
	entry_ref(const entry_ref &ref);
	~entry_ref();

	status_t set_name(const char *name);
	bool operator==(const entry_ref &ref) const;
	bool operator!=(const entry_ref &ref) const;
	entry_ref & operator=(const entry_ref &ref);

	dev_t device;
	ino_t directory;
	char *name;
};

