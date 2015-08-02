<p align='right'> <b>25/04/2010</b>
</p>


---


# Heap size of a Linux process #

Any optimisation task requires some kind of criteria. To optimize the memory footprint of Toped DB we need a simple and reliable measurement of the amount of heap memory allocated by the tool. After some research on the net and multiply tries with C libraries, external tolls, commands, scripts etc. I actually realised (surprise surprise!) that Kernel memory management these days appears to be quite a complex task and extracting the tiny bit of data that I needed out of the OS proves to be a real challenge. Besides it appeared that there are a variety of OS environmental factors that influence those reports - most of them not easily if at all controllable. A small snapshot of my adventures below

  * OS commands (ps, top) - see [Understanding memory usage on Linux](http://virtualthreads.blogspot.com/2006/02/understanding-memory-usage-on-linux.html)
  * reading `proc/<pid>/status` - not surprisingly it appears to be not quite different from the above
  * getrusage() - I manage to squeeze literally nothing about the memory out of this function - then I found [this](http://www.atarininja.org/index.py/tags/code). It appeared that vital parts of **struct rusage** fields are simply not implemented on my platform.
  * external tools/libraries - [valgrind](http://valgrind.org), [efence](http://directory.fsf.org/project/ElectricFence), [dmalloc](http://dmalloc.com) . They seem to be either too heavy or not quite up to this particular task.
  * scripts - most of what I found, not surprisingly is actually exploring the features already listed above. It's worth mentioning the script which seem to be relatively simple to create/install and generates descent reports (See the appendix at the bottom of the page). Still though it suffers the same lack of clarity "what's what" which of course is due to the overall Kernel memory management. Some interesting observations:
    * Heap is allocated on pages. It's virtually impossible to spot a creation of a single small object.
    * I couldn't find a way to distinguish between the heap used by the program itself, and the heap used by the linked libraries.
    * Heap is deallocated when required, not when a delete/free is executed. This often creates the impression that heap is an ever growing resource consumed by your program. When the unused heap will be claimed back by the OS and granted to another process is entirely due to the Kernel.

So - the task has to be approached from a different angle.

# Heap size of the Toped DB #
The old reliable DIY (Do it yourself) approach comes quite handy. In this instance it comes down to overwriting two C++ operators - **new** and **delete** for the base class of the DB. Thus with a relatively minimal changes we'll be able to track ourselves the overall amount of the heap memory allocated for the DB at any moment and note - the precision of this report does not depends on external factors. A good reference about overwriting new and delete can be found [here](http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=40). Surprisingly - it took me less than half an hour to get first reports. Then come some surprises. Here is a short test program. Although it might be trivial for some, the results appeared interesting for me.

```
#include <stdlib.h>
#include <iostream>
using namespace std;

class BaseClassNonVirtual {
   public:
                      BaseClassNonVirtual() {};
      static void*    operator new(size_t);
      static void     operator delete(void* p) {free(p);}
      int             nvfunc()  {return 0;}
};

class InhClassNonVirtual : public BaseClassNonVirtual {
   public:
                      InhClassNonVirtual() : BaseClassNonVirtual(), _data(0) {};
   private:
      int             _data;
};

class BaseClassVirtual {
   public:
                      BaseClassVirtual() {};
      static void*    operator new(size_t);
      static void     operator delete(void* p) {free(p);}
      virtual int     vfunc()  {return 0;}
};

class InhClassVirtual : public BaseClassVirtual {
   public:
                      InhClassVirtual() : BaseClassVirtual(), _data(0) {};
      virtual int     vfunc()  {return 1;}
   private:
      int             _data;
};

void* BaseClassNonVirtual::operator new(size_t size)
{
   cout << "Size of the object in bytes is: " << size << endl;
   void *p=malloc(size);
   return p;
}

void* BaseClassVirtual::operator new(size_t size)
{
   cout << "Size of the object in bytes is: " << size << endl;
   void *p=malloc(size);
   return p;
}

int main(void)
{
   BaseClassNonVirtual* pntA;
   InhClassNonVirtual*  pntB;
   cout << "=== BaseClassNonVirtual =============" << endl;
   pntA = new BaseClassNonVirtual();
   cout << "=== InhClassNonVirtual ==============" << endl;
   pntB = new InhClassNonVirtual();
   delete pntA;
   delete pntB;

   BaseClassVirtual* pntC;
   InhClassVirtual*  pntD;
   cout << "=== BaseClassVirtual ================" << endl;
   pntC = new BaseClassVirtual();
   cout << "=== InhClassVirtual =================" << endl;
   pntD = new InhClassVirtual();
   delete pntC;
   delete pntD;

}

```

When the above was launched on 64-bit Fedora 12 platform it reported:
```
=== BaseClassNonVirtual =============
Size of the object in bytes is: 1
=== InhClassNonVirtual ==============
Size of the object in bytes is: 4
=== BaseClassVirtual ================
Size of the object in bytes is: 8
=== InhClassVirtual =================
Size of the object in bytes is: 16
```

  1. An instance of a class with no data fields whatsoever is having a size of 1 byte. It's not clear what this byte is used for - most likely it's unused. The same size is reported if the class had a single data field of type char.
  1. If the class has virtual methods - its object size as well as the object size of the inheriting classes grows with 8 bytes. Is that a pointer to the virtual method table?
  1. The granularity follows the pattern 1-2-4-n\*8 where n > 0. It makes sense for 64 bit platform.

Some interesting rules can be drawn from the above:
  * For the classes without virtual methods
    * every byte might matter for the memory footprint if the overall size of the data fields is less than 8 bytes.
    * if overall size of the data fields is more than 8 bytes - it doesn't matter whether an extra field is of type bool or long - it will take another 8 bytes.
  * For the classes with virtual methods the granularity is 8 bytes - so even the first data field will add at least 8 bytes


---

# Appendix: smaps.pl #
This script can be found on the net, but with some broken links to the required perl modules. Here are they:
  * [Class-Member](http://search.cpan.org/~opi/Class-Member-1.6/)
  * [Smaps](http://search.cpan.org/~opi/Linux-Smaps-0.06/)

The script itself:
```
#!/usr/bin/perl
#smaps.pl
#typical usage is as follows :
# smaps.pl pid

use Linux::Smaps;

my $pid=shift @ARGV;
unless ($pid) {
        print "./smem.pl <pid>\n";
        exit 1;
}
my $map=Linux::Smaps->new($pid);
my @VMAs = $map->vmas;

format STDOUT =
VMSIZE:  @######## kb
$map->size
RSS:    @######## kb total
$map->rss
        @######## kb shared
$map->shared_clean + $map->shared_dirty
        @######## kb private clean
$map->private_clean
        @######## kb private dirty
$map->private_dirty
.

write;

printPrivateMappings ();
printSharedMappings ();

sub sharedMappings () {
    return grep { ($_->shared_clean  + $_->shared_dirty) > 0 } @VMAs;
}

sub privateMappings () {
    return grep { ($_->private_clean  + $_->private_dirty) > 0 } @VMAs;
}

sub printPrivateMappings ()
{
    $TYPE = "PRIVATE MAPPINGS";
    $^ = 'SECTION_HEADER';
    $~ = 'SECTION_ITEM';
    $- = 0;
    $= = 100000000;
    foreach  $vma (sort {-($a->private_dirty <=> $b->private_dirty)}
                                  privateMappings ()) {
        $size  = $vma->size;
        $dirty = $vma->private_dirty;
        $clean = $vma->private_clean;
        $file  = $vma->file_name;
        write;
    }
}

sub printSharedMappings ()
{
    $TYPE = "SHARED MAPPINGS";
    $^ = 'SECTION_HEADER';
    $~ = 'SECTION_ITEM';
    $- = 0;
    $= = 100000000;

    foreach  $vma (sort {-(($a->shared_clean + $a->shared_dirty)
                          <=>
                          ($b->shared_clean + $b->shared_dirty))}
                  sharedMappings ()) {

        $size  = $vma->size;
        $dirty = $vma->shared_dirty;
        $clean = $vma->shared_clean;
        $file  = $vma->file_name;
        write;
    }
}

format SECTION_HEADER =
@<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
$TYPE
@>>>>>>>>>> @>>>>>>>>>>  @>>>>>>>>>  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
"vmsize" "rss clean" "rss dirty" "file"
.

format SECTION_ITEM =
@####### kb @####### kb @####### kb  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
$size $clean $dirty $file
.
```