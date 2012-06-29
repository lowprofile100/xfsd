#include "xfsd.h"
#include "read_file.h"
#include "read_super.h"
#include "syscall.h"

#include "xfsd_types.h"
#include "xfs/xfs_types.h"
#include "xfs/xfs_inum.h"
#include "linux/rbtree.h"
#include "xfs/xfs_ag.h"
#include "xfs/xfs_dinode.h"
#include "xfs/xfs_bmap_btree.h"
#include "xfs/uuid.h"
#define __KERNEL__
#include "xfs/xfs_inode.h"
#undef __KERNEL__

#include "xfs/xfs_sb.h"

#include "xfs/xfs_da_btree.h"
#define __KERNEL__
#include "xfs/xfs_mount.h"
#undef __KERNEL__
#include "xfs/xfs_dir2_format.h"

/* 
 * For the function in xfs_dir2_sf.c
 */
#include "xfs/xfs_dir2_priv.h"

#include "tslib/tslib_types.h"

extern xfs_agi_t agi;
extern xfs_sb_t sb;

void
xfs_dinode_from_disk(
	xfs_icdinode_t		*to,
	xfs_dinode_t		*from)
{
	to->di_magic = be16_to_cpu(from->di_magic);
	to->di_mode = be16_to_cpu(from->di_mode);
	to->di_version = from ->di_version;
	to->di_format = from->di_format;
	to->di_onlink = be16_to_cpu(from->di_onlink);
	to->di_uid = be32_to_cpu(from->di_uid);
	to->di_gid = be32_to_cpu(from->di_gid);
	to->di_nlink = be32_to_cpu(from->di_nlink);
	to->di_projid_lo = be16_to_cpu(from->di_projid_lo);
	to->di_projid_hi = be16_to_cpu(from->di_projid_hi);
	mem_cpy(to->di_pad, from->di_pad, sizeof(to->di_pad));
	to->di_flushiter = be16_to_cpu(from->di_flushiter);
	to->di_atime.t_sec = be32_to_cpu(from->di_atime.t_sec);
	to->di_atime.t_nsec = be32_to_cpu(from->di_atime.t_nsec);
	to->di_mtime.t_sec = be32_to_cpu(from->di_mtime.t_sec);
	to->di_mtime.t_nsec = be32_to_cpu(from->di_mtime.t_nsec);
	to->di_ctime.t_sec = be32_to_cpu(from->di_ctime.t_sec);
	to->di_ctime.t_nsec = be32_to_cpu(from->di_ctime.t_nsec);
	to->di_size = be64_to_cpu(from->di_size);
	to->di_nblocks = be64_to_cpu(from->di_nblocks);
	to->di_extsize = be32_to_cpu(from->di_extsize);
	to->di_nextents = be32_to_cpu(from->di_nextents);
	to->di_anextents = be16_to_cpu(from->di_anextents);
	to->di_forkoff = from->di_forkoff;
	to->di_aformat	= from->di_aformat;
	to->di_dmevmask	= be32_to_cpu(from->di_dmevmask);
	to->di_dmstate	= be16_to_cpu(from->di_dmstate);
	to->di_flags	= be16_to_cpu(from->di_flags);
	to->di_gen	= be32_to_cpu(from->di_gen);
}

static xfs_ino_t rootino;
static xfs_daddr_t agi_root;
static char block_buf[4096];
static char *block_bufp;

int init_read_file_from_disk()
{
	init();
	rootino = sb.sb_rootino;
	agi_root = get_agi_root();
}

__TSLIB___uint64_t read_blocks( void *mem, xfs_daddr_t block, int nmeb, __TSLIB___uint64_t size)
{
	if ( size && size < sb.sb_blocksize * nmeb)
	{
		read_file_length( mem, block * sb.sb_blocksize, size, 1);
		return size;
	}
	else
	{
		read_file_length( mem, block * sb.sb_blocksize, sb.sb_blocksize, nmeb);
		return sb.sb_blocksize * nmeb;
	}
	return 0;
}

// Errors are not handled here.
int cache_block_from_disk( xfs_daddr_t block)
{
	read_blocks( ( void *)block_buf, block, 1, 0);
	block_bufp = block_buf;
	return 0;
}

int init_mem( void **mem, int size)
{
	*mem = block_bufp;
	block_bufp += size;

	return 0;
}

xfs_dinode_t *read_inode_relative( xfs_agino_t inode, xfs_icdinode_t *mem)
{
	xfs_agblock_t blocks;
	xfs_daddr_t inoblock;
	xfs_dinode_t *temp;
	xfs_daddr_t offset; 
	/* 
	 * There are something dealing with xfs_agino_t in xfs_inum.h.
	 * But they all rely on xfs_mount.
	 * So I will use my own.
	 * Only dealing with the inode in the first AG!!
	 */

	/* Get the start block of this ag.*/
	blocks = sb.sb_agblocks;
	blocks *= get_agi_seqno();

	/* I shouldn't rely on these globle varibles, use params instead. */
	inoblock = inode >> sb.sb_inopblog;
	inoblock += blocks;

	/* Read the whole block and put it into a local buffer. */
	cache_block_from_disk( inoblock);
	offset = inode & XFS_INO_MASK( sb.sb_inopblog);

	init_mem( ( void **) &temp, sb.sb_inodesize * offset); 		// Set offset in block.
	init_mem( ( void **) &temp, sb.sb_inodesize); 			// Read it out.
	xfs_dinode_from_disk( mem, temp); 				// Copy it.

	return temp;

	/*
	xfs_dinode_t temp;
	xfs_daddr_t nblock = agi_root;
	struct xfs_btree_block cur;
	while ( 1)
	{
		read_block( nblock);

		// In face we should use xfs_inobt_block_t, which is
		// acturelly xfs_btree_sblock_t, length is XFS_BTREE_SBLOCK_LEN
		init_mem( ( void *)&cur, XFS_BTREE_SBLOCK_LEN);

		if ( temp.bb_level)
		{
			size_t key_mem_size = sizeof( xfs_inobt_key_t) * temp.bbnumbers;
			xfs_inobt_key_t *keys;
			init_mem( &( void *)keys, key_mem_size);

			size_t ptr_mem_size = sizeof( xfs_inobt_key_t) * temp.bbnumbers;
			xfs_inobt_key_t *ptrs;
			init_mem( &( void *)ptrs, ptr_mem_size);

			int i = 0;
			while ( i < temp.bbnumbers && num < keys[i])
			{
				++i;
			}
		}
		else
		{
			size_t rec_mem_size = sizeof( xfs_inobt_rec_t) * temp.bbnumbers;
			xfs_inobt_rec_t *recs;
			init_mem( &( void *)recs, rec_mem_size);
			break;
		}
	}
	xfs_dinode_from_disk( &mem, &temp);
	*/
}

xfs_ino_t find_ino_by_path( const char *file_name)
{
	xfs_icdinode_t cur;
	xfs_dinode_t *raw;
	xfs_dir2_sf_hdr_t *hdrptr;
	xfs_dir2_sf_entry_t *entptr;
	__TSLIB___uint8_t count;
	const char *tail;
	int name_len;
	xfs_ino_t next_ino;
        next_ino = rootino;

	if ( *file_name != '/')
	{
		// Not supported yet.
		return 0;
	}

	eprint("move to dir: %s\n", file_name);

	/* Process to the end of the path.*/
	while ( *file_name)
	{
		/* Omit the first '/'. If this is the last one, then we get a dir.*/
		if ( !*++file_name)
		{
			break;
		}

		/* Get to next inode*/
		if ( ( raw = read_inode_relative( next_ino, &cur)) != NULL)
		{
			eprint("%x %x %x\n", (int)cur.di_magic, (int)cur.di_mode, (int)cur.di_version);

			/* If this is a dir. Only simple form is supported now. */
			if ( ( cur.di_mode & S_IFMT) == S_IFDIR || cur.di_format != XFS_DINODE_FMT_LOCAL)
			{
				hdrptr = ( xfs_dir2_sf_hdr_t *)XFS_DFORK_DPTR( raw);
				count = hdrptr->count;

				/* This struct is packed, please note it won't be usefull under windows. */
				entptr = xfs_dir2_sf_firstentry( hdrptr);
				eprint("get entry len %d, offset %d\n", (int)( entptr->namelen), (int)( entptr->offset.i[1]));

				tail = file_name;
				while ( *tail && *tail != '/')
				{
					++tail;
				}
				name_len = tail - file_name;

				while( count--)
				{
					if ( name_len == entptr->namelen && str_ncmp( entptr->name, file_name, name_len) == 0)
					{
						/* We find next file/dir here. */
						file_name = tail;
						next_ino = xfs_dir2_sfe_get_ino( hdrptr, entptr);
						break;
					}
					entptr = xfs_dir2_sf_nextentry( hdrptr, entptr);
				}

				if ( file_name != tail)
				{
					return 0; /* Not found. */
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			eprint("read ino error %lld\n", (long long)next_ino);
			return 0;
		}
	}

	return next_ino;
}

long read_file_from_disk( const char *file_name, void *mem, __TSLIB___uint64_t size)
{
	xfs_icdinode_t cur;
	xfs_dinode_t *raw;

	int cnt;
	xfs_bmbt_rec_t *rec;
	xfs_bmbt_rec_host_t host;
	xfs_bmbt_irec_t irec;

	xfs_ino_t next_ino;
        next_ino = find_ino_by_path( file_name);

	if ( next_ino <= 0)
	{
		return -1; // File not Found.
	}

	/* Now we start to read the content of the file. */
	eprint("Get first file block %lld\n", (long long)next_ino);
	if ( ( raw = read_inode_relative( next_ino, &cur)) != NULL)
	{
		if ( cur.di_mode & S_IFMT != S_IFREG || cur.di_format != XFS_DINODE_FMT_EXTENTS)
		{
			return -1;
		}
		else
		{
			rec = ( xfs_bmbt_rec_t *)XFS_DFORK_DPTR( raw);
			cnt = XFS_DFORK_NEXTENTS( raw, XFS_DATA_FORK);
			if ( size < be64_to_cpu( raw->di_size))
			{
				return 0; /* No enough space. */
			}
			else
			{
				size = be64_to_cpu( raw->di_size);
			}

			while ( cnt--)
			{
				host.l0 = be64_to_cpu( rec->l0);
				host.l1 = be64_to_cpu( rec->l1);
				xfs_bmbt_get_all( &host, &irec);

				eprint("%u %u %u\n", irec.br_startoff, irec.br_startblock, irec.br_blockcount);

				size -= read_blocks( ( char *)mem + irec.br_startoff, irec.br_startblock, irec.br_blockcount, size);
				rec++;
			}
		}
	}
	else
	{
		eprint("read ino error %lld\n", (long long)next_ino);
		return -1;
	}

	return 0;
}

int list_file( const char *path, char *buf)
{
	xfs_icdinode_t cur;
	xfs_dinode_t *raw;
	xfs_dir2_sf_hdr_t *hdrptr;
	xfs_dir2_sf_entry_t *entptr;

	xfs_ino_t next_ino;
	int i = -1;
	next_ino = find_ino_by_path( path);

	if ( next_ino > 0 && ( raw = read_inode_relative( next_ino, &cur)) != NULL)
	{
		if ( ( cur.di_mode & S_IFMT) != S_IFDIR || cur.di_format != XFS_DINODE_FMT_LOCAL)
		{
			return -1;
		}
		else
		{
			hdrptr = ( xfs_dir2_sf_hdr_t *)XFS_DFORK_DPTR( raw);

			for ( i = 0, entptr = xfs_dir2_sf_firstentry( hdrptr);
					i < hdrptr->count;
					i++, entptr = xfs_dir2_sf_nextentry( hdrptr, entptr))
			{
				mem_cpy( buf, (char *)( entptr->name), entptr->namelen);
				buf += entptr->namelen;
				*buf = '\0';
				++buf;
			}
		}
	}

	return i;
}
