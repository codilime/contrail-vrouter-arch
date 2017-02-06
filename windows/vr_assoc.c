#include "precomp.h"

#define MAP_SIZE 512

typedef void (*setterFunc)(struct vr_assoc*, const void*);
typedef BOOLEAN (*compareFunc)(struct vr_assoc*, const void*);
typedef int (*hashFunc)(const void*);

/*
 * A generated map of chars.
 * Has a field for every possible ASCII char (256 characters).
 * Maps all hyphens and alphanumeric characters into separate numbers, while the rest to 0.
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

static struct vr_assoc* name_map[MAP_SIZE];
static struct vr_assoc* ids_map[MAP_SIZE];

NDIS_IF_COUNTED_STRING vr_get_name_from_friendly_name(const NDIS_IF_COUNTED_STRING friendly)
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

struct vr_assoc* vr_get_assoc(struct vr_assoc** map, setterFunc setter, hashFunc hash, compareFunc cmp, const void* target)
{
    struct vr_assoc** field = map + hash(target);

    while (*field != NULL)
    {
        if (cmp(*field, target) == 1)
            break;
        field = &((*field)->next);
    }

    if (*field == NULL)
    {
        *field = ExAllocatePoolWithTag(NonPagedPool, sizeof(struct vr_assoc), SxExtAllocationTag);
        RtlZeroMemory(*field, sizeof(struct vr_assoc));
        setter(*field, target);
    }

    return (*field);
}

void vr_delete_assoc(struct vr_assoc** map, hashFunc hash, compareFunc cmp, const void* target)
{
    // This is a pointer to a pointer. Therefore at the start in points to the pointer in the hasharray and then it points to the "next" field of the previous list entry.
    struct vr_assoc** field = map + hash(target);

    while (*field != NULL)
    {
        if (cmp(*field, target) == 1)
            break;
        field = &((*field)->next);
    }

    if (*field == NULL)
        return; // Such entry did not exist

    ExFreePoolWithTag(*field, SxExtAllocationTag);
    *field = (*field)->next;
}

static void setter_name(struct vr_assoc* entry, NDIS_IF_COUNTED_STRING* interface_name)
{
    entry->string = *interface_name;
}

static int hash_name(const NDIS_IF_COUNTED_STRING* interface_name)
{
    int hash = 0;
    int i = interface_name->Length;
    const WCHAR* str = interface_name->String;

    while (i-- != 0)
    {
        hash ^= char_map[(*str++) % 256] * 5;
    }

    return hash;
}

static BOOLEAN cmp_name(struct vr_assoc* entry, const NDIS_IF_COUNTED_STRING* target)
{
    return (entry->string.Length == target->Length && wcsncmp(entry->string.String, target->String, target->Length) == 0);
}

struct vr_interface* vr_get_assoc_name(const NDIS_IF_COUNTED_STRING interface_name)
{
    return vr_get_assoc(name_map, (setterFunc)setter_name, (hashFunc)hash_name, (compareFunc)cmp_name, &interface_name)->interface;
}

void vr_set_assoc_oid_name(const NDIS_IF_COUNTED_STRING interface_name, struct vr_interface* interface)
{
    struct vr_assoc* element = vr_get_assoc(name_map, (setterFunc)setter_name, (hashFunc)hash_name, (compareFunc)cmp_name, &interface_name);

    element->interface = interface;
    element->sources |= VR_OID_SOURCE;
    return;
}

void vr_delete_assoc_name(const NDIS_IF_COUNTED_STRING interface_name)
{
    vr_delete_assoc(name_map, (hashFunc)hash_name, (compareFunc)cmp_name, &interface_name);
}

struct ids_pair {
    NDIS_SWITCH_PORT_ID port;
    NDIS_SWITCH_NIC_INDEX nic;
};

static void setter_ids(struct vr_assoc* entry, const struct ids_pair* target)
{
    entry->port_id = target->port;
    entry->nic_index = target->nic;
}

static int hash_ids(const struct ids_pair* pair)
{
    int hash = (pair->port + pair->nic) % MAP_SIZE; //NIC is almost always 0 and port grows incrementally, so this should be a pretty good hash

    return hash;
}

static BOOLEAN cmp_ids(struct vr_assoc* entry, const struct ids_pair* target)
{
    return (entry->port_id == target->port && entry->nic_index == target->nic);
}

struct vr_interface* vr_get_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index)
{
    struct ids_pair pair;
    pair.nic = nic_index;
    pair.port = port_id;

    return vr_get_assoc(ids_map, (setterFunc)setter_ids, (hashFunc)hash_ids, (compareFunc)cmp_ids, &pair)->interface;
}

void vr_set_assoc_oid_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index, struct vr_interface* interface)
{
    struct ids_pair pair;
    pair.nic = nic_index;
    pair.port = port_id;

    struct vr_assoc* element = vr_get_assoc(ids_map, (setterFunc)setter_ids, (hashFunc)hash_ids, (compareFunc)cmp_ids, &pair);

    element->interface = interface;
    element->sources |= VR_OID_SOURCE;
    return;
}

void vr_delete_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index)
{
    struct ids_pair pair;
    pair.nic = nic_index;
    pair.port = port_id;

    vr_delete_assoc(name_map, (hashFunc)hash_ids, (compareFunc)cmp_ids, &pair);
}

void vr_assoc_destroy(struct vr_assoc** map)
{
    struct vr_assoc* next_element;
    struct vr_assoc* element;

    for (int i = 0; i < MAP_SIZE; i++)
    {
        element = map[i];
        while (element)
        {
            next_element = element->next;
            // Not deleting vr_interface contained inside, because it will be done by dp-core (vr_interface.c)
            ExFreePoolWithTag(element, SxExtAllocationTag);
            element = next_element;
        }
        map[i] = NULL;
    }
}

void vr_clean_assoc()
{
    vr_assoc_destroy(ids_map);
    vr_assoc_destroy(name_map);
}
