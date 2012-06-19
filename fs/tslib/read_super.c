#include "read_super.h"
#include "xfsd_types.h"
#include "syscall.h"

#include "xfs/uuid.h"
#include "xfs/xfs_types.h"
#include "xfs/xfs_sb.h"

#include "xfs/xfs_inum.h"
#include "linux/rbtree.h"
#include "xfs/xfs_ag.h"

#include "xfs/xfs_ialloc_btree.h"
#include "xfs/xfs_alloc_btree.h"
#include "xfs/xfs_bmap_btree.h"
#include "xfs/xfs_btree.h"

static xfs_dsb_t dsb;
xfs_sb_t sb;
static xfs_agf_t agf;
static xfs_agi_t agi;
static xfs_agfl_t agfl[10];
static struct xfs_btree_block bblock;

void
xfs_sb_from_disk(
		xfs_sb_t *to,
		xfs_dsb_t	*from)
{
	to->sb_magicnum = be32_to_cpu(from->sb_magicnum);
	to->sb_blocksize = be32_to_cpu(from->sb_blocksize);
	to->sb_dblocks = be64_to_cpu(from->sb_dblocks);
	to->sb_rblocks = be64_to_cpu(from->sb_rblocks);
	to->sb_rextents = be64_to_cpu(from->sb_rextents);
	mem_cpy(&to->sb_uuid, &from->sb_uuid, sizeof(to->sb_uuid));
	to->sb_logstart = be64_to_cpu(from->sb_logstart);
	to->sb_rootino = be64_to_cpu(from->sb_rootino);
	to->sb_rbmino = be64_to_cpu(from->sb_rbmino);
	to->sb_rsumino = be64_to_cpu(from->sb_rsumino);
	to->sb_rextsize = be32_to_cpu(from->sb_rextsize);
	to->sb_agblocks = be32_to_cpu(from->sb_agblocks);
	to->sb_agcount = be32_to_cpu(from->sb_agcount);
	to->sb_rbmblocks = be32_to_cpu(from->sb_rbmblocks);
	to->sb_logblocks = be32_to_cpu(from->sb_logblocks);
	to->sb_versionnum = be16_to_cpu(from->sb_versionnum);
	to->sb_sectsize = be16_to_cpu(from->sb_sectsize);
	to->sb_inodesize = be16_to_cpu(from->sb_inodesize);
	to->sb_inopblock = be16_to_cpu(from->sb_inopblock);
	mem_cpy(&to->sb_fname, &from->sb_fname, sizeof(to->sb_fname));
	to->sb_blocklog = from->sb_blocklog;
	to->sb_sectlog = from->sb_sectlog;
	to->sb_inodelog = from->sb_inodelog;
	to->sb_inopblog = from->sb_inopblog;
	to->sb_agblklog = from->sb_agblklog;
	to->sb_rextslog = from->sb_rextslog;
	to->sb_inprogress = from->sb_inprogress;
	to->sb_imax_pct = from->sb_imax_pct;
	to->sb_icount = be64_to_cpu(from->sb_icount);
	to->sb_ifree = be64_to_cpu(from->sb_ifree);
	to->sb_fdblocks = be64_to_cpu(from->sb_fdblocks);
	to->sb_frextents = be64_to_cpu(from->sb_frextents);
	to->sb_uquotino = be64_to_cpu(from->sb_uquotino);
	to->sb_gquotino = be64_to_cpu(from->sb_gquotino);
	to->sb_qflags = be16_to_cpu(from->sb_qflags);
	to->sb_flags = from->sb_flags;
	to->sb_shared_vn = from->sb_shared_vn;
	to->sb_inoalignmt = be32_to_cpu(from->sb_inoalignmt);
	to->sb_unit = be32_to_cpu(from->sb_unit);
	to->sb_width = be32_to_cpu(from->sb_width);
	to->sb_dirblklog = from->sb_dirblklog;
	to->sb_logsectlog = from->sb_logsectlog;
	to->sb_logsectsize = be16_to_cpu(from->sb_logsectsize);
	to->sb_logsunit = be32_to_cpu(from->sb_logsunit);
	to->sb_features2 = be32_to_cpu(from->sb_features2);
	to->sb_bad_features2 = be32_to_cpu(from->sb_bad_features2);
}

int read_block( int offset, void *mem, int nmeb)
{
	// Here is a bug: we cannot seek files larger that 2GB.
	if (  ( seek_file_set( offset * ( long) sb.sb_blocksize)) == -1)
	{
		return -1;
	}

	return read_file( mem, sb.sb_block_size, nmeb);
}

int init()
{
	if ( open_file( "tslib/xfs.lib", "r"))
	{
		return -1;
	}
	else
	{
		seek_file_set( 0);
		read_file( ( void *) &dsb, sizeof( sb), 1);
		xfs_sb_from_disk( &sb, &dsb);

		seek_file_set( sb.sb_sectsize);
		read_file( ( void *)&agf, sizeof( agf), 1);

		seek_file_set( sb.sb_sectsize * 2);
		read_file( ( void *)&agi, sizeof( agi), 1);

		seek_file_set( sb.sb_sectsize * 3);
		read_file( ( void *)agfl, sizeof( xfs_agfl_t), be32_to_cpu( agf.agf_flcount));
	}
	return 0;
}

void get_sb_magic( char * magic)
{
	char *cur = ( char *)&(sb.sb_magicnum);
	mem_cpy( magic, cur, 4);
}

unsigned int get_dsb_magic_int()
{
	return dsb.sb_magicnum;
}

int get_sbs_count()
{
	return XFS_SB_NUM_BITS;
}

xfs_sb_t get_sb()
{
	return sb;
}

int get_dsb_size()
{
	return sizeof( xfs_dsb_t);
}

int get_sb_size()
{
	return sizeof( xfs_sb_t);
}

int get_sb_features2()
{
	return sb.sb_features2;
}

int get_sb_sectsize()
{
	return sb.sb_sectsize;
}

int get_agf_magic( char * magic)
{
	char *cur = ( char *)&(agf.agf_magicnum);
	mem_cpy( magic, cur, 4);
}

int get_agf_free_block( int count)
{
	int flcount = be32_to_cpu( agf.agf_flcount);
	int ret = 0;
	if ( count <= flcount)
	{
		ret = count + be32_to_cpu( agf.agf_flfirst);
		ret = be32_to_cpu( agfl[ret].agfl_bno[0]);
	}
	return ret;
}

int get_agf_flcount()
{
	return be32_to_cpu( agf.agf_flcount);
}

int get_agf_versionnum()
{
	return be32_to_cpu( agf.agf_versionnum);
}


int get_sb_ifree()
{
	return sb.sb_ifree;
}

xfs_ino_t get_sb_rootino()
{
	return sb.sb_rootino;
}

xfs_daddr_t get_agi_root()
{
	return ( xfs_daddr_t)be32_to_cpu( agi.agi_root);
}
