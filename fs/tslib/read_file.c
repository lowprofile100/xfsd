#include "xfsd.h"
#include "read_file.h"
#include "read_super.h"
#include "syscall.h"

#include "xfs/xfs_inum.h"
#include "linux/rbtree.h"
#include "xfs/xfs_ag.h"
#include "xfs/xfs_dinode.h"
#include "xfs/xfs_inode.h"

#include "xfs_dir2_format.h"

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
static char *block_buf[4096];
static char *block_bufp;

int init_read_file()
{
	rootino = sb.sb_rootino;
	agi_root = agi.agi_root;
}

// Errors are not handled here.
void read_block( xfs_daddr_t block)
{
	read_file_length( ( void *)block_buf, block * sb.sb_blocksize, sb.sb_blocksize, 1);
	block_bufp = block_buf;

	return 0;
}

int init_mem( void **mem, int size)
{
	*mem = block_bufp;
	block_bufp += size;

	return 0;
}

int read_inode_relative( xfs_agino_t inode, xfs_icdinode_t *mem)
{
	/* 
	 * There are something dealing with xfs_agino_t in xfs_inum.h.
	 * But they all rely on xfs_mount.
	 * So I will use my own.
	 * Only dealing with the inode in the first AG!!
	 */

	/* Get the start block of this ag.*/
	xfs_agblock_t blocks = sb.sb_agblocks;
	blocks *= agi.agi_seqno;

	/* I shouldn't rely on these globle varibles, use params instead. */
	xfs_daddr_t inoblock = inode >> sb.sb_inopblog;
	inoblock += blocks;

	/* Read the whole block and put it into a local buffer. */
	read_block( inoblock);
	xfs_dinode_t *temp;
	xfs_daddr_t offset = inode & XFS_INO_MASK( sb.sb_inopblog);

	init_mem( &( void *)temp, sizeof( xfs_dinode_t) * offset); 	// Set offset in block.
	init_mem( &( void *)temp, sizeof( xfs_dinode_t)); 		// Read it out.
	xfs_dinode_from_disk( mem, temp); 				// Copy it.

	return 0;

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

int read_file( const char *file_name, void *mem, size_t size)
{
	if ( *file_name != '/')
	{
		// Not supported yet.
		return -3;
	}

	++file_name;
	int len = str_len( file_name);
	xfs_icdinode_t cur;
	/* Get to the root inode*/
	if ( read_inode_relative( rootino, &cur) == 0)
	{
		/* Process to the end of the path.*/
		while ( len)
		{
			/* If this is a dir. */
			if ( ( cur.di_mode & S_IFMT) == S_IFDIR)
			{
				/* Only simple form is supported now. */
				if ( cur.di_format != XFS_DINODE_FMT_LOCAL)
				{
					return -3;
				}
				else
				{
					xfs_dir2_sf_hdr_t *hdrptr = XFS_DFORK_DPTP( &cur);
					__TSLIB___uint8_t count = hdrptr->count;
					__TSLIB___uint8_t i8count = hdrptr->i8count;
					int tot = -1;
					int len;
					if ( ( !count) ^ ( !i8count))
					{
						tot = count + i8count;
						len = count ? 4 : 8;
					}

					if ( tot == -1)
					{
						return -2;
					}
					else
					{
						xfs_dir2_sf_entry_t *entptr = xfs_dir2_sf_firstentry( hdrptr);
						while( tot--)
						{
							ino = xfs_dir2_sf_get_ino( sfp, sfep);
						}
					}
				}
			}
			int last;
		}
	}
	else
	{
		printf("read ino error %lld\n", (int)rootino);
		return -1;
	}
	return 0;
}
