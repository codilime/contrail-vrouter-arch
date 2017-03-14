#include "precomp.h"

extern PSX_SWITCH_OBJECT SxSwitchObject;
NDIS_RW_LOCK_EX* name_lock;
NDIS_RW_LOCK_EX* ids_lock;

#define MAP_SIZE 512
#define HASH_ERROR ((unsigned int)(-1))

struct criteria {
    const char *name;
    NDIS_SWITCH_PORT_ID port_id;
    NDIS_SWITCH_NIC_INDEX nic_index;
};

typedef void (*setterFunc)(struct vr_assoc*, const struct criteria*);
typedef BOOLEAN (*compareFunc)(struct vr_assoc*, const struct criteria*);
typedef unsigned int (*hashFunc)(const struct criteria*);

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

NDIS_STATUS vr_get_name_from_friendly_name(
    NDIS_IF_COUNTED_STRING friendly,
    char *name,
    size_t name_buffer_size)
{
    // Ensure that string in `friendly` is null-terminated
    unsigned int pos_past_end = friendly.Length;
    if (pos_past_end >= IF_MAX_STRING_SIZE + 1) {
        return NDIS_STATUS_FAILURE;
    }
    friendly.String[pos_past_end] = '\0';

    UNICODE_STRING friendly_unicode_str;
    ANSI_STRING friendly_ansi_str;
    NTSTATUS status;

    RtlUnicodeStringInit(&friendly_unicode_str, friendly.String);
    status = RtlUnicodeStringToAnsiString(&friendly_ansi_str, &friendly_unicode_str, TRUE);
    if (status != STATUS_SUCCESS) {
        return NDIS_STATUS_FAILURE;
    }

    int i = friendly_ansi_str.Length;
    // The names are in format of "Container Port a30f213f"
    // To be the most accurate, get the last "word", speparated by a space
    while (friendly_ansi_str.Buffer[--i] != ' ') {
        if (i == 0) {
            // Name is not conforming to our standards, must be not a container port
            return NDIS_STATUS_FAILURE;
        }
    }

    PCHAR src = friendly_ansi_str.Buffer + i + 1;
    size_t src_max_bytes = friendly_ansi_str.Length - i - 1;
    status = RtlStringCbCopyNA(name, name_buffer_size, src, src_max_bytes);
    if (status != STATUS_SUCCESS) {
        return NDIS_STATUS_FAILURE;
    }

    RtlFreeAnsiString(&friendly_ansi_str);

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS vr_assoc_set_string(struct vr_assoc *entry, const char* new_assoc_string)
{
    if (entry == NULL || new_assoc_string == NULL) {
        return NDIS_STATUS_FAILURE;
    }

    NTSTATUS copy_status = RtlStringCbCopyA(entry->string, sizeof(entry->string), new_assoc_string);
    if (NT_SUCCESS(copy_status)) {
        return NDIS_STATUS_SUCCESS;
    } else {
        return NDIS_STATUS_FAILURE;
    }
}

struct vr_assoc* vr_get_assoc(struct vr_assoc** map, setterFunc setter, hashFunc hash, compareFunc cmp, const struct criteria* params)
{
    unsigned int calculated_hash = hash(params);
    if (calculated_hash >= MAP_SIZE) {
        return NULL;
    }

    struct vr_assoc** field = map + hash(params);

    while (*field != NULL)
    {
        if (cmp(*field, params) == 1)
            break;
        field = &((*field)->next);
    }

    if (*field == NULL)
    {
        *field = ExAllocatePoolWithTag(NonPagedPoolNx, sizeof(struct vr_assoc), SxExtAllocationTag);
        if (*field == NULL) {
            return NULL;
        }
        RtlZeroMemory(*field, sizeof(struct vr_assoc));
        setter(*field, params);
    }

    return (*field);
}

void vr_delete_assoc(struct vr_assoc** map, hashFunc hash, compareFunc cmp, const struct criteria* params)
{
    // This is a pointer to a pointer. Therefore at the start in points to the pointer in the hasharray and then it points to the "next" field of the previous list entry.
    struct vr_assoc** field = map + hash(params);

    while (*field != NULL)
    {
        if (cmp(*field, params) == 1)
            break;
        field = &((*field)->next);
    }

    if (*field == NULL)
        return; // Such entry did not exist

    struct vr_assoc *tmp = (*field)->next;
    ExFreePoolWithTag(*field, SxExtAllocationTag);
    *field = tmp;
}

static void setter_name(struct vr_assoc* entry, const struct criteria* params)
{
    if (entry) {
        vr_assoc_set_string(entry, params->name);
        entry->nic_index = 0;
        entry->port_id = 0;
    }
}

static unsigned int hash_name(const struct criteria* params)
{
    size_t name_length;
    NTSTATUS name_length_status = RtlStringCbLengthA(params->name, VR_ASSOC_STRING_SIZE, &name_length);
    if (!NT_SUCCESS(name_length_status)) {
        return HASH_ERROR;
    }

    int hash = 0;
    int i = 0;
    for (i = 0; i < name_length; ++i) {
        hash ^= char_map[(params->name[i]) % 256] * 5;
    }

    return hash;
}

static BOOLEAN cmp_name(struct vr_assoc* entry, const struct criteria* params)
{
    NTSTATUS status;

    size_t entry_string_length;
    status = RtlStringCbLengthA(entry->string, VR_ASSOC_STRING_SIZE, &entry_string_length);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    size_t params_name_length;
    status = RtlStringCbLengthA(params->name, VR_ASSOC_STRING_SIZE, &params_name_length);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    return entry_string_length == params_name_length && !strncmp(entry->string, params->name, VR_ASSOC_STRING_MAX_LEN);
}

struct vr_assoc* vr_get_assoc_by_name(const char *interface_name)
{
    struct criteria params;
    params.name = interface_name;

    LOCK_STATE_EX lock;

    // This is because the requested element can be lazy-created
    NdisAcquireRWLockWrite(name_lock, &lock, 0);
    struct vr_assoc* ret = vr_get_assoc(name_map, setter_name, hash_name, cmp_name, &params);
    NdisReleaseRWLock(name_lock, &lock);

    return ret;
}

void vr_set_assoc_by_name(const char *interface_name, struct vr_interface* interface)
{
    struct criteria params;
    params.name = interface_name;

    LOCK_STATE_EX lock;

    NdisAcquireRWLockWrite(name_lock, &lock, 0);

    struct vr_assoc* element = vr_get_assoc(name_map, setter_name, hash_name, cmp_name, &params);

    if (element) {
        element->interface = interface;
        element->sources |= VR_OID_SOURCE;
    }

    NdisReleaseRWLock(name_lock, &lock);

    return;
}

void vr_delete_assoc_by_name(const char *interface_name)
{
    struct criteria params;
    params.name = interface_name;

    LOCK_STATE_EX lock;

    NdisAcquireRWLockWrite(name_lock, &lock, 0);

    vr_delete_assoc(name_map, hash_name, cmp_name, &params);

    NdisReleaseRWLock(name_lock, &lock);
}

static void setter_ids(struct vr_assoc* entry, const struct criteria* params)
{
    if (entry) {
        entry->nic_index = params->nic_index;
        entry->port_id = params->port_id;
        entry->interface = NULL;
    }
}

static unsigned int hash_ids(const struct criteria* params)
{
    int hash = (params->port_id + params->nic_index) % MAP_SIZE; //NIC is almost always 0 and port grows incrementally, so this should be a pretty good hash

    return hash;
}

static BOOLEAN cmp_ids(struct vr_assoc* entry, const struct criteria* params)
{
    return (entry->port_id == params->port_id && entry->nic_index == params->nic_index);
}

struct vr_assoc* vr_get_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index)
{
    struct criteria params;
    params.nic_index = nic_index;
    params.port_id = port_id;

    LOCK_STATE_EX lock;

    // This is because the requested element can be lazy-created
    NdisAcquireRWLockWrite(ids_lock, &lock, 0);

    struct vr_assoc* ret = vr_get_assoc(ids_map, setter_ids, hash_ids, cmp_ids, &params);

    NdisReleaseRWLock(ids_lock, &lock);

    return ret;
}

void vr_set_assoc_oid_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index, struct vr_interface* interface)
{
    struct criteria params;
    params.nic_index = nic_index;
    params.port_id = port_id;

    LOCK_STATE_EX lock;

    NdisAcquireRWLockWrite(ids_lock, &lock, 0);

    struct vr_assoc* element = vr_get_assoc(ids_map, setter_ids, hash_ids, cmp_ids, &params);

    if (element) {
        element->interface = interface;
        element->sources |= VR_OID_SOURCE;
    }

    NdisReleaseRWLock(ids_lock, &lock);
    return;
}

void vr_delete_assoc_ids(const NDIS_SWITCH_PORT_ID port_id, const NDIS_SWITCH_NIC_INDEX nic_index)
{
    struct criteria params;
    params.nic_index = nic_index;
    params.port_id = port_id;

    LOCK_STATE_EX lock;

    NdisAcquireRWLockWrite(ids_lock, &lock, 0);
    vr_delete_assoc(ids_map, hash_ids, cmp_ids, &params);
    NdisReleaseRWLock(ids_lock, &lock);
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
    LOCK_STATE_EX lock;

    NdisAcquireRWLockWrite(ids_lock, &lock, 0);
    vr_assoc_destroy(ids_map);
    NdisReleaseRWLock(ids_lock, &lock);

    NdisAcquireRWLockWrite(name_lock, &lock, 0);
    vr_assoc_destroy(name_map);
    NdisReleaseRWLock(name_lock, &lock);

    NdisFreeRWLock(name_lock);
    NdisFreeRWLock(ids_lock);
}

int vr_init_assoc()
{
    name_lock = NdisAllocateRWLock(SxSwitchObject->NdisFilterHandle);
    if (!name_lock) {
        return VR_INIT_ASSOC_FAILED;
    }

    ids_lock = NdisAllocateRWLock(SxSwitchObject->NdisFilterHandle);
    if (!ids_lock) {
        return VR_INIT_ASSOC_FAILED;
    }

    return VR_INIT_ASSOC_OK;
}
