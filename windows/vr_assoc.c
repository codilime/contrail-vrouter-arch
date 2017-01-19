#include "precomp.h"
#include "vr_windows.h"

#define MAP_SIZE 512

/*
 * A generated map of chars.
 * Has a field for every possible ASCII char (256 characters).
 * Maps all hyphens and alphanumeric characters into sepeparate numbers, while the rest to 0.
 * Thanks to that, no data has to be sacrificed for ASCII characters that never appear in interface names.
 */
const static unsigned char char_map[256] = {
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      53,     0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      27,     28,     29,     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,
	42,     43,     44,     45,     46,     47,     48,     49,     50,     51,     52,     0,      0,      0,      0,      0,
	0,      1,      2,      3,      4,      5,      6,      7,      8,      9,      10,     11,     12,     13,     14,     15,
	16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,
};

static struct vr_assoc* map[MAP_SIZE];

NDIS_IF_COUNTED_STRING vr_get_name_from_friendly_name(NDIS_IF_COUNTED_STRING friendly)
{
	NDIS_IF_COUNTED_STRING ret;

	int i = friendly.Length;
	// The names are in format of "Container Port a30f213f"
	// To be the most accurate, get the last "word", speparated by a space
	while (friendly.String[--i] != L' ');

	wcscpy_s(ret.String, friendly.Length - i + 1, friendly.String + i + 1);
	ret.Length = (USHORT)(friendly.Length - i + 1);

	return ret;
}

int vr_hash_nic(NDIS_IF_COUNTED_STRING interface_name)
{
	int hash = 0;
	int i = interface_name.Length;
	WCHAR* str = interface_name.String;

	while (i-- != 0)
	{
		hash ^= char_map[(*str++) % 256] * 5;
	}

	return hash;
}

struct vr_assoc* vr_get_assoc(NDIS_IF_COUNTED_STRING interface_name)
{
	struct vr_assoc** field = map + vr_hash_nic(interface_name);

	while (*field != NULL)
	{
		if ((*field)->string.Length == interface_name.Length && wcsncmp((*field)->string.String, interface_name.String, interface_name.Length) == 0)
			break;
		field = &((*field)->next);
	}

	if (*field == NULL)
	{
		*field = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct vr_assoc), SxExtAllocationTag);
		(*field)->string = interface_name;
		(*field)->next = NULL;
	}

	return *field;
}

void vr_set_assoc_oid(NDIS_IF_COUNTED_STRING interface_name, NDIS_SWITCH_PORT_ID port_id)
{
	struct vr_assoc* element = vr_get_assoc(interface_name);

	element->port_id = port_id;
	element->sources |= VR_OID_SOURCE;
	return;
}

void vr_assoc_destroy()
{
	struct vr_assoc* next_element;
	struct vr_assoc* element;

	for (int i = 0; i < MAP_SIZE; i++)
	{
		element = map[i];
		while (element)
		{
			next_element = element->next;
			ExFreePoolWithTag(element, SxExtAllocationTag);
			element = next_element;
		}
	}
}
