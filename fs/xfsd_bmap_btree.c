/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "xfsd.h"
#include "xfsd_types.h"
#include "linux/rbtree.h"

#include "xfs/xfs_fs.h"
#include "xfs/xfs_types.h"
#include "xfs/xfs_bit.h"
#include "xfs/xfs_log.h"
#include "xfs/xfs_inum.h"
#include "xfs/xfs_sb.h"
#include "xfs/xfs_ag.h"
#include "xfs/xfs_bmap_btree.h"
#include "xfs/xfs_alloc_btree.h"
#include "xfs/xfs_ialloc_btree.h"
#include "xfs/xfs_dinode.h"
#define __KERNEL__
#include "xfs/xfs_inode.h"
#undef __KERNEL__
#include "xfs/xfs_trans.h"
#define __KERNEL__
#include "xfs/xfs_mount.h"
#undef __KERNEL__
#include "xfs/xfs_inode_item.h"
#include "xfs/xfs_alloc.h"
#include "xfs/xfs_btree.h"
#include "xfs/xfs_itable.h"
#include "xfs/xfs_bmap.h"
#include "xfs/xfs_error.h"
#include "xfs/xfs_quota.h"

/*
 * Determine the extent state.
 */
/* ARGSUSED */
STATIC xfs_exntst_t
xfs_extent_state(
	xfs_filblks_t		blks,
	int			extent_flag)
{
	if (extent_flag) {
		ASSERT(blks != 0);	/* saved for DMIG */
		return XFS_EXT_UNWRITTEN;
	}
	return XFS_EXT_NORM;
}

/*
 * Convert on-disk form of btree root to in-memory form.
 */
void
xfs_bmdr_to_bmbt(
	struct xfs_mount	*mp,
	xfs_bmdr_block_t	*dblock,
	int			dblocklen,
	struct xfs_btree_block	*rblock,
	int			rblocklen)
{
	int			dmxr;
	xfs_bmbt_key_t		*fkp;
	__be64			*fpp;
	xfs_bmbt_key_t		*tkp;
	__be64			*tpp;

	rblock->bb_magic = cpu_to_be32(XFS_BMAP_MAGIC);
	rblock->bb_level = dblock->bb_level;
	ASSERT(be16_to_cpu(rblock->bb_level) > 0);
	rblock->bb_numrecs = dblock->bb_numrecs;
	rblock->bb_u.l.bb_leftsib = cpu_to_be64(NULLDFSBNO);
	rblock->bb_u.l.bb_rightsib = cpu_to_be64(NULLDFSBNO);
	dmxr = xfs_bmdr_maxrecs(mp, dblocklen, 0);
	fkp = XFS_BMDR_KEY_ADDR(dblock, 1);
	tkp = XFS_BMBT_KEY_ADDR(mp, rblock, 1);
	fpp = XFS_BMDR_PTR_ADDR(dblock, 1, dmxr);
	tpp = XFS_BMAP_BROOT_PTR_ADDR(mp, rblock, 1, rblocklen);
	dmxr = be16_to_cpu(dblock->bb_numrecs);
	mem_cpy(tkp, fkp, sizeof(*fkp) * dmxr);
	mem_cpy(tpp, fpp, sizeof(*fpp) * dmxr);
}

/*
 * Convert a compressed bmap extent record to an uncompressed form.
 * This code must be in sync with the routines xfs_bmbt_get_startoff,
 * xfs_bmbt_get_startblock, xfs_bmbt_get_blockcount and xfs_bmbt_get_state.
 */
STATIC void
__xfs_bmbt_get_all(
		__uint64_t l0,
		__uint64_t l1,
		xfs_bmbt_irec_t *s)
{
	int	ext_flag;
	xfs_exntst_t st;

	ext_flag = (int)(l0 >> (64 - BMBT_EXNTFLAG_BITLEN));
	s->br_startoff = ((xfs_fileoff_t)l0 &
			   xfs_mask64lo(64 - BMBT_EXNTFLAG_BITLEN)) >> 9;
#if XFS_BIG_BLKNOS
	s->br_startblock = (((xfs_fsblock_t)l0 & xfs_mask64lo(9)) << 43) |
			   (((xfs_fsblock_t)l1) >> 21);
#else
#ifdef DEBUG
	{
		xfs_dfsbno_t	b;

		b = (((xfs_dfsbno_t)l0 & xfs_mask64lo(9)) << 43) |
		    (((xfs_dfsbno_t)l1) >> 21);
		ASSERT((b >> 32) == 0 || isnulldstartblock(b));
		s->br_startblock = (xfs_fsblock_t)b;
	}
#else	/* !DEBUG */
	s->br_startblock = (xfs_fsblock_t)(((xfs_dfsbno_t)l1) >> 21);
#endif	/* DEBUG */
#endif	/* XFS_BIG_BLKNOS */
	s->br_blockcount = (xfs_filblks_t)(l1 & xfs_mask64lo(21));
	/* This is xfs_extent_state() in-line */
	if (ext_flag) {
		ASSERT(s->br_blockcount != 0);	/* saved for DMIG */
		st = XFS_EXT_UNWRITTEN;
	} else
		st = XFS_EXT_NORM;
	s->br_state = st;
}

void
xfs_bmbt_get_all(
	xfs_bmbt_rec_host_t *r,
	xfs_bmbt_irec_t *s)
{
	__xfs_bmbt_get_all(r->l0, r->l1, s);
}

/*
 * Extract the blockcount field from an in memory bmap extent record.
 */
xfs_filblks_t
xfs_bmbt_get_blockcount(
	xfs_bmbt_rec_host_t	*r)
{
	return (xfs_filblks_t)(r->l1 & xfs_mask64lo(21));
}

/*
 * Extract the startblock field from an in memory bmap extent record.
 */
xfs_fsblock_t
xfs_bmbt_get_startblock(
	xfs_bmbt_rec_host_t	*r)
{
#if XFS_BIG_BLKNOS
	return (((xfs_fsblock_t)r->l0 & xfs_mask64lo(9)) << 43) |
	       (((xfs_fsblock_t)r->l1) >> 21);
#else
#ifdef DEBUG
	xfs_dfsbno_t	b;

	b = (((xfs_dfsbno_t)r->l0 & xfs_mask64lo(9)) << 43) |
	    (((xfs_dfsbno_t)r->l1) >> 21);
	ASSERT((b >> 32) == 0 || isnulldstartblock(b));
	return (xfs_fsblock_t)b;
#else	/* !DEBUG */
	return (xfs_fsblock_t)(((xfs_dfsbno_t)r->l1) >> 21);
#endif	/* DEBUG */
#endif	/* XFS_BIG_BLKNOS */
}

/*
 * Extract the startoff field from an in memory bmap extent record.
 */
xfs_fileoff_t
xfs_bmbt_get_startoff(
	xfs_bmbt_rec_host_t	*r)
{
	return ((xfs_fileoff_t)r->l0 &
		 xfs_mask64lo(64 - BMBT_EXNTFLAG_BITLEN)) >> 9;
}

xfs_exntst_t
xfs_bmbt_get_state(
	xfs_bmbt_rec_host_t	*r)
{
	int	ext_flag;

	ext_flag = (int)((r->l0) >> (64 - BMBT_EXNTFLAG_BITLEN));
	return xfs_extent_state(xfs_bmbt_get_blockcount(r),
				ext_flag);
}

/*
 * Extract the blockcount field from an on disk bmap extent record.
 */
xfs_filblks_t
xfs_bmbt_disk_get_blockcount(
	xfs_bmbt_rec_t	*r)
{
	return (xfs_filblks_t)(be64_to_cpu(r->l1) & xfs_mask64lo(21));
}

/*
 * Extract the startoff field from a disk format bmap extent record.
 */
xfs_fileoff_t
xfs_bmbt_disk_get_startoff(
	xfs_bmbt_rec_t	*r)
{
	return ((xfs_fileoff_t)be64_to_cpu(r->l0) &
		 xfs_mask64lo(64 - BMBT_EXNTFLAG_BITLEN)) >> 9;
}


/*
 * Set all the fields in a bmap extent record from the arguments.
 */
void
xfs_bmbt_set_allf(
	xfs_bmbt_rec_host_t	*r,
	xfs_fileoff_t		startoff,
	xfs_fsblock_t		startblock,
	xfs_filblks_t		blockcount,
	xfs_exntst_t		state)
{
	int		extent_flag = (state == XFS_EXT_NORM) ? 0 : 1;

	ASSERT(state == XFS_EXT_NORM || state == XFS_EXT_UNWRITTEN);
	ASSERT((startoff & xfs_mask64hi(64-BMBT_STARTOFF_BITLEN)) == 0);
	ASSERT((blockcount & xfs_mask64hi(64-BMBT_BLOCKCOUNT_BITLEN)) == 0);

#if XFS_BIG_BLKNOS
	ASSERT((startblock & xfs_mask64hi(64-BMBT_STARTBLOCK_BITLEN)) == 0);

	r->l0 = ((xfs_bmbt_rec_base_t)extent_flag << 63) |
		((xfs_bmbt_rec_base_t)startoff << 9) |
		((xfs_bmbt_rec_base_t)startblock >> 43);
	r->l1 = ((xfs_bmbt_rec_base_t)startblock << 21) |
		((xfs_bmbt_rec_base_t)blockcount &
		(xfs_bmbt_rec_base_t)xfs_mask64lo(21));
#else	/* !XFS_BIG_BLKNOS */
	if (isnullstartblock(startblock)) {
		r->l0 = ((xfs_bmbt_rec_base_t)extent_flag << 63) |
			((xfs_bmbt_rec_base_t)startoff << 9) |
			 (xfs_bmbt_rec_base_t)xfs_mask64lo(9);
		r->l1 = xfs_mask64hi(11) |
			  ((xfs_bmbt_rec_base_t)startblock << 21) |
			  ((xfs_bmbt_rec_base_t)blockcount &
			   (xfs_bmbt_rec_base_t)xfs_mask64lo(21));
	} else {
		r->l0 = ((xfs_bmbt_rec_base_t)extent_flag << 63) |
			((xfs_bmbt_rec_base_t)startoff << 9);
		r->l1 = ((xfs_bmbt_rec_base_t)startblock << 21) |
			 ((xfs_bmbt_rec_base_t)blockcount &
			 (xfs_bmbt_rec_base_t)xfs_mask64lo(21));
	}
#endif	/* XFS_BIG_BLKNOS */
}

/*
 * Set all the fields in a bmap extent record from the uncompressed form.
 */
void
xfs_bmbt_set_all(
	xfs_bmbt_rec_host_t *r,
	xfs_bmbt_irec_t	*s)
{
	xfs_bmbt_set_allf(r, s->br_startoff, s->br_startblock,
			     s->br_blockcount, s->br_state);
}


/*
 * Set all the fields in a disk format bmap extent record from the arguments.
 */
void
xfs_bmbt_disk_set_allf(
	xfs_bmbt_rec_t		*r,
	xfs_fileoff_t		startoff,
	xfs_fsblock_t		startblock,
	xfs_filblks_t		blockcount,
	xfs_exntst_t		state)
{
	int			extent_flag = (state == XFS_EXT_NORM) ? 0 : 1;

	ASSERT(state == XFS_EXT_NORM || state == XFS_EXT_UNWRITTEN);
	ASSERT((startoff & xfs_mask64hi(64-BMBT_STARTOFF_BITLEN)) == 0);
	ASSERT((blockcount & xfs_mask64hi(64-BMBT_BLOCKCOUNT_BITLEN)) == 0);

#if XFS_BIG_BLKNOS
	ASSERT((startblock & xfs_mask64hi(64-BMBT_STARTBLOCK_BITLEN)) == 0);

	r->l0 = cpu_to_be64(
		((xfs_bmbt_rec_base_t)extent_flag << 63) |
		 ((xfs_bmbt_rec_base_t)startoff << 9) |
		 ((xfs_bmbt_rec_base_t)startblock >> 43));
	r->l1 = cpu_to_be64(
		((xfs_bmbt_rec_base_t)startblock << 21) |
		 ((xfs_bmbt_rec_base_t)blockcount &
		  (xfs_bmbt_rec_base_t)xfs_mask64lo(21)));
#else	/* !XFS_BIG_BLKNOS */
	if (isnullstartblock(startblock)) {
		r->l0 = cpu_to_be64(
			((xfs_bmbt_rec_base_t)extent_flag << 63) |
			 ((xfs_bmbt_rec_base_t)startoff << 9) |
			  (xfs_bmbt_rec_base_t)xfs_mask64lo(9));
		r->l1 = cpu_to_be64(xfs_mask64hi(11) |
			  ((xfs_bmbt_rec_base_t)startblock << 21) |
			  ((xfs_bmbt_rec_base_t)blockcount &
			   (xfs_bmbt_rec_base_t)xfs_mask64lo(21)));
	} else {
		r->l0 = cpu_to_be64(
			((xfs_bmbt_rec_base_t)extent_flag << 63) |
			 ((xfs_bmbt_rec_base_t)startoff << 9));
		r->l1 = cpu_to_be64(
			((xfs_bmbt_rec_base_t)startblock << 21) |
			 ((xfs_bmbt_rec_base_t)blockcount &
			  (xfs_bmbt_rec_base_t)xfs_mask64lo(21)));
	}
#endif	/* XFS_BIG_BLKNOS */
}

/*
 * Set all the fields in a bmap extent record from the uncompressed form.
 */
STATIC void
xfs_bmbt_disk_set_all(
	xfs_bmbt_rec_t	*r,
	xfs_bmbt_irec_t *s)
{
	xfs_bmbt_disk_set_allf(r, s->br_startoff, s->br_startblock,
				  s->br_blockcount, s->br_state);
}

/*
 * Set the blockcount field in a bmap extent record.
 */
void
xfs_bmbt_set_blockcount(
	xfs_bmbt_rec_host_t *r,
	xfs_filblks_t	v)
{
	ASSERT((v & xfs_mask64hi(43)) == 0);
	r->l1 = (r->l1 & (xfs_bmbt_rec_base_t)xfs_mask64hi(43)) |
		  (xfs_bmbt_rec_base_t)(v & xfs_mask64lo(21));
}

/*
 * Set the startblock field in a bmap extent record.
 */
void
xfs_bmbt_set_startblock(
	xfs_bmbt_rec_host_t *r,
	xfs_fsblock_t	v)
{
#if XFS_BIG_BLKNOS
	ASSERT((v & xfs_mask64hi(12)) == 0);
	r->l0 = (r->l0 & (xfs_bmbt_rec_base_t)xfs_mask64hi(55)) |
		  (xfs_bmbt_rec_base_t)(v >> 43);
	r->l1 = (r->l1 & (xfs_bmbt_rec_base_t)xfs_mask64lo(21)) |
		  (xfs_bmbt_rec_base_t)(v << 21);
#else	/* !XFS_BIG_BLKNOS */
	if (isnullstartblock(v)) {
		r->l0 |= (xfs_bmbt_rec_base_t)xfs_mask64lo(9);
		r->l1 = (xfs_bmbt_rec_base_t)xfs_mask64hi(11) |
			  ((xfs_bmbt_rec_base_t)v << 21) |
			  (r->l1 & (xfs_bmbt_rec_base_t)xfs_mask64lo(21));
	} else {
		r->l0 &= ~(xfs_bmbt_rec_base_t)xfs_mask64lo(9);
		r->l1 = ((xfs_bmbt_rec_base_t)v << 21) |
			  (r->l1 & (xfs_bmbt_rec_base_t)xfs_mask64lo(21));
	}
#endif	/* XFS_BIG_BLKNOS */
}

/*
 * Set the startoff field in a bmap extent record.
 */
void
xfs_bmbt_set_startoff(
	xfs_bmbt_rec_host_t *r,
	xfs_fileoff_t	v)
{
	ASSERT((v & xfs_mask64hi(9)) == 0);
	r->l0 = (r->l0 & (xfs_bmbt_rec_base_t) xfs_mask64hi(1)) |
		((xfs_bmbt_rec_base_t)v << 9) |
		  (r->l0 & (xfs_bmbt_rec_base_t)xfs_mask64lo(9));
}

/*
 * Set the extent state field in a bmap extent record.
 */
void
xfs_bmbt_set_state(
	xfs_bmbt_rec_host_t *r,
	xfs_exntst_t	v)
{
	ASSERT(v == XFS_EXT_NORM || v == XFS_EXT_UNWRITTEN);
	if (v == XFS_EXT_NORM)
		r->l0 &= xfs_mask64lo(64 - BMBT_EXNTFLAG_BITLEN);
	else
		r->l0 |= xfs_mask64hi(BMBT_EXNTFLAG_BITLEN);
}

/*
 * Convert in-memory form of btree root to on-disk form.
 */
void
xfs_bmbt_to_bmdr(
	struct xfs_mount	*mp,
	struct xfs_btree_block	*rblock,
	int			rblocklen,
	xfs_bmdr_block_t	*dblock,
	int			dblocklen)
{
	int			dmxr;
	xfs_bmbt_key_t		*fkp;
	__be64			*fpp;
	xfs_bmbt_key_t		*tkp;
	__be64			*tpp;

	ASSERT(rblock->bb_magic == cpu_to_be32(XFS_BMAP_MAGIC));
	ASSERT(rblock->bb_u.l.bb_leftsib == cpu_to_be64(NULLDFSBNO));
	ASSERT(rblock->bb_u.l.bb_rightsib == cpu_to_be64(NULLDFSBNO));
	ASSERT(rblock->bb_level != 0);
	dblock->bb_level = rblock->bb_level;
	dblock->bb_numrecs = rblock->bb_numrecs;
	dmxr = xfs_bmdr_maxrecs(mp, dblocklen, 0);
	fkp = XFS_BMBT_KEY_ADDR(mp, rblock, 1);
	tkp = XFS_BMDR_KEY_ADDR(dblock, 1);
	fpp = XFS_BMAP_BROOT_PTR_ADDR(mp, rblock, 1, rblocklen);
	tpp = XFS_BMDR_PTR_ADDR(dblock, 1, dmxr);
	dmxr = be16_to_cpu(dblock->bb_numrecs);
	mem_cpy(tkp, fkp, sizeof(*fkp) * dmxr);
	mem_cpy(tpp, fpp, sizeof(*fpp) * dmxr);
}

STATIC void
xfs_bmbt_update_cursor(
	struct xfs_btree_cur	*src,
	struct xfs_btree_cur	*dst)
{
	ASSERT((dst->bc_private.b.firstblock != NULLFSBLOCK) ||
	       (dst->bc_private.b.ip->i_d.di_flags & XFS_DIFLAG_REALTIME));
	ASSERT(dst->bc_private.b.flist == src->bc_private.b.flist);

	dst->bc_private.b.allocated += src->bc_private.b.allocated;
	dst->bc_private.b.firstblock = src->bc_private.b.firstblock;

	src->bc_private.b.allocated = 0;
}

STATIC int
xfs_bmbt_get_minrecs(
	struct xfs_btree_cur	*cur,
	int			level)
{
	if (level == cur->bc_nlevels - 1) {
		struct xfs_ifork	*ifp;

		ifp = XFS_IFORK_PTR(cur->bc_private.b.ip,
				    cur->bc_private.b.whichfork);

		return xfs_bmbt_maxrecs(cur->bc_mp,
					ifp->if_broot_bytes, level == 0) / 2;
	}

	return cur->bc_mp->m_bmap_dmnr[level != 0];
}

int
xfs_bmbt_get_maxrecs(
	struct xfs_btree_cur	*cur,
	int			level)
{
	if (level == cur->bc_nlevels - 1) {
		struct xfs_ifork	*ifp;

		ifp = XFS_IFORK_PTR(cur->bc_private.b.ip,
				    cur->bc_private.b.whichfork);

		return xfs_bmbt_maxrecs(cur->bc_mp,
					ifp->if_broot_bytes, level == 0);
	}

	return cur->bc_mp->m_bmap_dmxr[level != 0];

}

/*
 * Get the maximum records we could store in the on-disk format.
 *
 * For non-root nodes this is equivalent to xfs_bmbt_get_maxrecs, but
 * for the root node this checks the available space in the dinode fork
 * so that we can resize the in-memory buffer to match it.  After a
 * resize to the maximum size this function returns the same value
 * as xfs_bmbt_get_maxrecs for the root node, too.
 */
STATIC int
xfs_bmbt_get_dmaxrecs(
	struct xfs_btree_cur	*cur,
	int			level)
{
	if (level != cur->bc_nlevels - 1)
		return cur->bc_mp->m_bmap_dmxr[level != 0];
	return xfs_bmdr_maxrecs(cur->bc_mp, cur->bc_private.b.forksize,
				level == 0);
}

STATIC void
xfs_bmbt_init_key_from_rec(
	union xfs_btree_key	*key,
	union xfs_btree_rec	*rec)
{
	key->bmbt.br_startoff =
		cpu_to_be64(xfs_bmbt_disk_get_startoff(&rec->bmbt));
}

STATIC void
xfs_bmbt_init_rec_from_key(
	union xfs_btree_key	*key,
	union xfs_btree_rec	*rec)
{
	ASSERT(key->bmbt.br_startoff != 0);

	xfs_bmbt_disk_set_allf(&rec->bmbt, be64_to_cpu(key->bmbt.br_startoff),
			       0, 0, XFS_EXT_NORM);
}

STATIC void
xfs_bmbt_init_rec_from_cur(
	struct xfs_btree_cur	*cur,
	union xfs_btree_rec	*rec)
{
	xfs_bmbt_disk_set_all(&rec->bmbt, &cur->bc_rec.b);
}

STATIC void
xfs_bmbt_init_ptr_from_cur(
	struct xfs_btree_cur	*cur,
	union xfs_btree_ptr	*ptr)
{
	ptr->l = 0;
}

STATIC __int64_t
xfs_bmbt_key_diff(
	struct xfs_btree_cur	*cur,
	union xfs_btree_key	*key)
{
	return (__int64_t)be64_to_cpu(key->bmbt.br_startoff) -
				      cur->bc_rec.b.br_startoff;
}

/*
 * Calculate number of records in a bmap btree block.
 */
int
xfs_bmbt_maxrecs(
	struct xfs_mount	*mp,
	int			blocklen,
	int			leaf)
{
	blocklen -= XFS_BMBT_BLOCK_LEN(mp);

	if (leaf)
		return blocklen / sizeof(xfs_bmbt_rec_t);
	return blocklen / (sizeof(xfs_bmbt_key_t) + sizeof(xfs_bmbt_ptr_t));
}

/*
 * Calculate number of records in a bmap btree inode root.
 */
int
xfs_bmdr_maxrecs(
	struct xfs_mount	*mp,
	int			blocklen,
	int			leaf)
{
	blocklen -= sizeof(xfs_bmdr_block_t);

	if (leaf)
		return blocklen / sizeof(xfs_bmdr_rec_t);
	return blocklen / (sizeof(xfs_bmdr_key_t) + sizeof(xfs_bmdr_ptr_t));
}
