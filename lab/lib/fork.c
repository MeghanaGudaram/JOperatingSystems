// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void _pgfault_upcall(void);

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	//cprintf("hi..%x\n",(uvpt[PGNUM((uint32_t)addr)]& (PTE_W|PTE_COW)));
	
	if((err & FEC_WR) ==0)
	panic("FEC_WR present: %e at address [%08x] (not a write)", err, addr);
	
	if((uvpt[PGNUM((uint32_t)addr)] & (PTE_W|PTE_COW)) == 0)
	panic("Permissions are not write and COW!");
	
	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	//panic("pgfault not implemented");
	
	void *vap = (void *) ROUNDDOWN((uint32_t) addr, PGSIZE);
	
	if((r=sys_page_alloc(0, (void *)PFTEMP, PTE_P|PTE_W|PTE_U)) < 0)
	panic("Allocating page failed in pgfault: %e",r);
	
	memmove((void *)PFTEMP, vap, PGSIZE);
	if ((r = sys_page_map(0, (void *)PFTEMP, 0, vap, PTE_P|PTE_W|PTE_U)) < 0)
	panic("Error in pgfault, sys_page_map: %e", r);
	
	
	if ((r = sys_page_unmap(0, (void *)PFTEMP)) < 0)
	panic("Error in pgfault, sys_page_unmap: %e", r);

}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	int perm = PTE_COW | PTE_P;
	// LAB 4: Your code here.
	//panic("duppage not implemented");
	
	void *va = (void *) (pn << PGSHIFT);
	
	if(uvpt[pn] & PTE_SHARE)
	{
		if((r = sys_page_map(thisenv->env_id, va, envid, va, uvpt[pn] & PTE_SYSCALL)) < 0)
		{
			panic("error in duppage share child: %e",r);
		}
	}
	else if(uvpt[pn] & PTE_COW || uvpt[pn] & PTE_W)
	{
		if (uvpt[pn] & PTE_U)
			perm |= PTE_U;
		if((r = sys_page_map(thisenv->env_id, va, envid, va, perm)) < 0)
		{
			panic("error in duppage child: %e",r);
		}
		
		if((r = sys_page_map(thisenv->env_id, va, thisenv->env_id, va, perm)) < 0)
		{
			panic("error in duppage parent: %e",r);
		}
	}
	else // if((uvpt[pn] & ~PTE_U) == 0)
	{
		if((r = sys_page_map(thisenv->env_id, va, envid, va, uvpt[pn] & PTE_SYSCALL)) < 0)
		{
			panic("error in read only duppage child: %e",r);
		}
	}
	

	return 0;
	
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented");
	envid_t envId = 5,r;
	
	set_pgfault_handler(pgfault);
	//cprintf("o..%d ",envId);
	if((envId=sys_exofork()) < 0)
	{
		panic("Could not fork: %e",envId);
	}
	//cprintf("hi..fork %d & %x\n",envId,(thisenv->env_id));
	if (envId == 0) 
	{
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	for(uint32_t pn = PGNUM(UTEXT); pn < PGNUM(UXSTACKTOP - PGSIZE); pn++)
	{
		if (!(uvpd[PDX(pn << PGSHIFT)] & PTE_P)) continue;
		if (!(uvpt[pn] & PTE_P)) continue;
		
		duppage(envId, pn);	
	}
	
	if ((r = sys_page_alloc(envId, (void*)(UXSTACKTOP-PGSIZE), PTE_P | PTE_U | PTE_W)) < 0)
		panic("sys_page_alloc: error %e\n", r);
	if ((r = sys_env_set_pgfault_upcall(envId, thisenv->env_pgfault_upcall)) < 0)
		panic("sys_env_set_pgfault_upcall: error %e\n", r);
	// Start the child environment running
	
	if ((r = sys_env_set_status(envId, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);
	
	return envId;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
