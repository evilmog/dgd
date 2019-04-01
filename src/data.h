/*
 * This file is part of DGD, https://github.com/dworkin/dgd
 * Copyright (C) 1993-2010 Dworkin B.V.
 * Copyright (C) 2010-2019 DGD Authors (see the commit log for details)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

struct strref {
    String *str;		/* string value */
    Dataspace *data;		/* dataspace this string is in */
    Uint ref;			/* # of refs */
};

struct arrref {
    Array *arr;			/* array value */
    Dataplane *plane;		/* value plane this array is in */
    Dataspace *data;		/* dataspace this array is in */
    short state;		/* state of mapping */
    Uint ref;			/* # of refs */
};

struct Value {
    char type;			/* value type */
    bool modified;		/* dirty bit */
    uindex oindex;		/* index in object table */
    union {
	Int number;		/* number */
	Uint objcnt;		/* object creation count */
	String *string;		/* string */
	Array *array;		/* array or mapping */
    };
};

struct dcallout {
    Uint time;			/* time of call */
    unsigned short mtime;	/* time of call milliseconds */
    uindex nargs;		/* number of arguments */
    Value val[4];		/* function name, 3 direct arguments */
};

# define co_prev	time
# define co_next	nargs

struct Dataplane {
    Int level;			/* dataplane level */

    short flags;		/* modification flags */
    long schange;		/* # string changes */
    long achange;		/* # array changes */
    long imports;		/* # array imports */

    Value *original;		/* original variables */
    arrref alocal;		/* primary of new local arrays */
    arrref *arrays;		/* i/o? arrays */
    Array::Backup *achunk;	/* chunk of array backup info */
    strref *strings;		/* i/o? string constant table */
    class coptable *coptab;	/* callout patch table */

    Dataplane *prev;		/* previous in per-dataspace linked list */
    Dataplane *plist;		/* next in per-level linked list */
};

struct Dataspace {
    Dataspace *prev, *next;	/* swap list */
    Dataspace *gcprev, *gcnext;	/* garbage collection list */

    Dataspace *iprev;		/* previous in import list */
    Dataspace *inext;		/* next in import list */

    Sector *sectors;		/* o vector of sectors */
    Sector nsectors;		/* o # sectors */

    short flags;		/* various bitflags */
    Control *ctrl;		/* control block */
    uindex oindex;		/* object this dataspace belongs to */

    unsigned short nvariables;	/* o # variables */
    Value *variables;		/* i/o variables */
    struct svalue *svariables;	/* o svariables */
    Uint varoffset;		/* o offset of variables in data space */

    Uint narrays;		/* i/o # arrays */
    Uint eltsize;		/* o total size of array elements */
    struct sarray *sarrays;	/* o sarrays */
    Uint *saindex;		/* o sarrays index */
    struct svalue *selts;	/* o sarray elements */
    Array alist;		/* array linked list sentinel */
    Uint arroffset;		/* o offset of array table in data space */

    Uint nstrings;		/* i/o # strings */
    Uint strsize;		/* o total size of string text */
    struct sstring *sstrings;	/* o sstrings */
    Uint *ssindex;		/* o sstrings index */
    char *stext;		/* o sstrings text */
    Uint stroffset;		/* o offset of string table */

    uindex ncallouts;		/* # callouts */
    uindex fcallouts;		/* free callout list */
    dcallout *callouts;		/* callouts */
    struct scallout *scallouts;	/* o scallouts */
    Uint cooffset;		/* offset of callout table */

    Dataplane base;		/* basic value plane */
    Dataplane *plane;		/* current value plane */

    struct parser *parser;	/* parse_string data */
};

# define THISPLANE(a)		((a)->plane == (a)->data->plane)
# define SAMEPLANE(d1, d2)	((d1)->plane->level == (d2)->plane->level)

extern void		d_init		 ();
extern void		d_init_conv	 (bool);

extern Dataspace       *d_new_dataspace  (Object*);
extern Dataspace       *d_load_dataspace (Object*);
extern void		d_ref_dataspace  (Dataspace*);

extern void		d_new_variables	 (Control*, Value*);
extern Value	       *d_get_variable	 (Dataspace*, unsigned int);
extern Value	       *d_get_elts	 (Array*);
extern void		d_get_callouts	 (Dataspace*);

extern Sector		d_swapout	 (unsigned int);
extern void		d_upgrade_mem	 (Object*, Object*);
extern Dataspace       *d_restore_data	 (Object*, Uint*,
					  void(*)(char*, Sector*, Uint, Uint));
extern void		d_restore_obj	 (Object*, Uint, Uint*, bool, bool);
extern void		d_converted	 ();

extern void		d_free_dataspace (Dataspace*);

extern void		d_new_plane	(Dataspace*, Int);
extern void		d_commit_plane	(Int, Value*);
extern void		d_discard_plane	(Int);
extern Array::Backup  **d_commit_arr	(Array*, Dataplane*, Dataplane*);
extern void		d_discard_arr	(Array*, Dataplane*);

extern void		d_ref_imports	(Array*);
extern void		d_assign_var	(Dataspace*, Value*, Value*);
extern Value	       *d_get_extravar	(Dataspace*);
extern void		d_set_extravar	(Dataspace*, Value*);
extern void		d_wipe_extravar	(Dataspace*);
extern void		d_assign_elt	(Dataspace*, Array*, Value*, Value*);
extern void		d_change_map	(Array*);

extern uindex		d_new_call_out	(Dataspace*, String*, Int,
					   unsigned int, Frame*, int);
extern Int		d_del_call_out	(Dataspace*, Uint, unsigned short*);
extern String	       *d_get_call_out	(Dataspace*, unsigned int, Frame*,
					   int*);
extern Array	       *d_list_callouts	(Dataspace*, Dataspace*);

extern void		d_set_varmap	(Control*, unsigned short*);
extern void		d_upgrade_data	(Dataspace*, unsigned int,
					   unsigned short*, Object*);
extern void		d_upgrade_clone	(Dataspace*);
extern Object	       *d_upgrade_lwobj	(Array*, Object*);
extern void		d_export	();

extern void		d_del_dataspace	(Dataspace*);


/* bit values for dataspace->flags */
# define DATA_STRCMP		0x03	/* strings compressed */

/* bit values for dataspace->plane->flags */
# define MOD_ALL		0x3f
# define MOD_VARIABLE		0x01	/* variable changed */
# define MOD_ARRAY		0x02	/* array element changed */
# define MOD_ARRAYREF		0x04	/* array reference changed */
# define MOD_STRINGREF		0x08	/* string reference changed */
# define MOD_CALLOUT		0x10	/* callout changed */
# define MOD_NEWCALLOUT		0x20	/* new callout added */
# define PLANE_MERGE		0x40	/* merge planes on commit */
# define MOD_SAVE		0x80	/* save on next full swapout */

# define ARR_MOD		0x80000000L	/* in arrref->ref */

# define AR_UNCHANGED		0	/* mapping unchanged */
# define AR_CHANGED		1	/* mapping changed */
