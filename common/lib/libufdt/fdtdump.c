/*
 * fdtdump.c - Contributed by Pantelis Antoniou <pantelis.antoniou AT gmail.com>
 */

#include "com_type.h"
#include "string.h"
#include "ctype.h"
#include "debug.h"

#include "libfdt_env.h"
#include "fdt.h"

#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))
#define PALIGN(p, a)	((void *)(ALIGN((unsigned long)(p), (a))))
#define GET_CELL(p)	(p += 4, *((const uint32_t *)(p-4)))

int is_printable_string(const void *data, int len)
{
        const char *s = data;
        const char *ss, *se;

        /* zero length is not */
        if (len == 0)
                return 0;

        /* must terminate with zero */
        if (s[len - 1] != '\0')
                return 0;

        se = s + len;

        while (s < se) {
                ss = s;
                while (s < se && *s && isprint(*s))
                        s++;

                /* not zero, or not done yet */
                if (*s != '\0' || s == ss)
                        return 0;

                s++;
        }

        return 1;
}


static void print_data(const char *data, int len)
{
	int i;
	const char *p = data;

	/* no data, don't print */
	if (len == 0)
		return;

	if (is_printable_string(data, len)) {
		NOTICE(" = \"%s\"", (const char *)data);
	} else if ((len % 4) == 0) {
		NOTICE(" = <");
		for (i = 0; i < len; i += 4)
			NOTICE("0x%08x%s", fdt32_to_cpu(GET_CELL(p)),
			       i < (len - 4) ? " " : "");
		NOTICE(">");
	} else {
		NOTICE(" = [");
		for (i = 0; i < len; i++)
			NOTICE("%02x%s", *p++, i < len - 1 ? " " : "");
		NOTICE("]");
	}
}

static void print_align(int n)
{
	int i = 0;
	for(i = 0; i < n; i++)
		NOTICE(" ");
}

void dump_blob(void *blob)
{
	struct fdt_header *bph = blob;
	uint32_t off_mem_rsvmap = fdt32_to_cpu(bph->off_mem_rsvmap);
	uint32_t off_dt = fdt32_to_cpu(bph->off_dt_struct);
	uint32_t off_str = fdt32_to_cpu(bph->off_dt_strings);
	struct fdt_reserve_entry *p_rsvmap =
		(struct fdt_reserve_entry *)((char *)blob + off_mem_rsvmap);
	const char *p_struct = (const char *)blob + off_dt;
	const char *p_strings = (const char *)blob + off_str;
	uint32_t version = fdt32_to_cpu(bph->version);
	uint32_t totalsize = fdt32_to_cpu(bph->totalsize);
	uint32_t tag;
	const char *p, *s, *t;
	int depth, sz, shift;
	int i;
	uint64_t addr, size;

	depth = 0;
	shift = 4;

	NOTICE("/dts-v1/;\n");
	NOTICE("// magic:\t\t0x%x\n", fdt32_to_cpu(bph->magic));
	NOTICE("// totalsize:\t\t0x%x (%d)\n", totalsize, totalsize);
	NOTICE("// off_dt_struct:\t0x%x\n", off_dt);
	NOTICE("// off_dt_strings:\t0x%x\n", off_str);
	NOTICE("// off_mem_rsvmap:\t0x%x\n", off_mem_rsvmap);
	NOTICE("// version:\t\t%d\n", version);
	NOTICE("// last_comp_version:\t%d\n",
	       fdt32_to_cpu(bph->last_comp_version));
	if (version >= 2)
		NOTICE("// boot_cpuid_phys:\t0x%x\n",
		       fdt32_to_cpu(bph->boot_cpuid_phys));

	if (version >= 3)
		NOTICE("// size_dt_strings:\t0x%x\n",
		       fdt32_to_cpu(bph->size_dt_strings));
	if (version >= 17)
		NOTICE("// size_dt_struct:\t0x%x\n",
		       fdt32_to_cpu(bph->size_dt_struct));
	NOTICE("\n");

	for (i = 0; ; i++) {
		addr = fdt64_to_cpu(p_rsvmap[i].address);
		size = fdt64_to_cpu(p_rsvmap[i].size);
		if (addr == 0 && size == 0)
			break;

		NOTICE("/memreserve/ %llx %llx;\n",
		       (unsigned long long)addr, (unsigned long long)size);
	}

	p = p_struct;
	while ((tag = fdt32_to_cpu(GET_CELL(p))) != FDT_END) {

		/* NOTICE("tag: 0x%08x (%d)\n", tag, p - p_struct); */

		if (tag == FDT_BEGIN_NODE) {
			s = p;
			p = PALIGN(p + strlen(s) + 1, 4);

			if (*s == '\0')
				s = "/";

			print_align(depth * shift);
			NOTICE("%s%s {\n", "", s);

			depth++;
			continue;
		}

		if (tag == FDT_END_NODE) {
			depth--;

			print_align(depth * shift);
			NOTICE("%s};\n", "");
			continue;
		}

		if (tag == FDT_NOP) {
			print_align(depth * shift);
			NOTICE("%s// [NOP]\n", "");
			continue;
		}

		if (tag != FDT_PROP) {
			print_align(depth * shift);
			ERR("%s ** Unknown tag 0x%08x\n", "", tag);
			break;
		}
		sz = fdt32_to_cpu(GET_CELL(p));
		s = p_strings + fdt32_to_cpu(GET_CELL(p));
		if (version < 16 && sz >= 8)
			p = PALIGN(p, 8);
		t = p;

		p = PALIGN(p + sz, 4);

		print_align(depth * shift);
		NOTICE("%s%s", "", s);
		print_data(t, sz);
		NOTICE(";\n");
	}
}
