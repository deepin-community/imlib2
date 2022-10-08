#include "common.h"

#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file.h"

int
__imlib_IsRealFile(const char *s)
{
   struct stat         st;

   return (stat(s, &st) != -1) && (S_ISREG(st.st_mode));
}

char               *
__imlib_FileKey(const char *file)
{
   char               *newfile;

   newfile = malloc(strlen(file) + 1);
   if (!newfile)
      return NULL;
   newfile[0] = 0;
   {
      char               *p1, *p2;
      int                 go;

      go = 0;
      p1 = (char *)file;
      p2 = newfile;
      while (p1[0])
        {
           if (go)
             {
                p2[0] = p1[0];
                p2++;
             }
           if ((p1[0] == ':') && (p1[1] != ':'))
              go = 1;
           if ((p1[0] == ':') && (p1[1] == ':'))
              p1++;
           p1++;
        }
      p2[0] = p1[0];
   }
   if (newfile[0])
      return newfile;
   else
      free(newfile);
   return NULL;
}

char               *
__imlib_FileRealFile(const char *file)
{
   char               *newfile;

   newfile = malloc(strlen(file) + 1);
   if (!newfile)
      return NULL;
   newfile[0] = 0;
   {
      char               *p1, *p2;

      p1 = (char *)file;
      p2 = newfile;
      while (p1[0])
        {
           if (p1[0] == ':')
             {
                if (p1[1] == ':')
                  {
                     p2[0] = ':';
                     p2++;
                     p1++;
                  }
                else
                  {
                     p2[0] = 0;
                     return newfile;
                  }
             }
           else
             {
                p2[0] = p1[0];
                p2++;
             }
           p1++;
        }
      p2[0] = p1[0];
   }
   return newfile;
}

const char         *
__imlib_FileExtension(const char *file)
{
   const char         *p, *s;
   int                 ch;

   if (!file)
      return NULL;

   for (p = s = file; (ch = *s) != 0; s++)
     {
        if (ch == '.' || ch == '/')
           p = s + 1;
     }
   return *p != '\0' ? p : NULL;
}

int
__imlib_FileStat(const char *file, struct stat *st)
{
   if ((!file) || (!*file))
      return -1;

   return stat(file, st);
}

int
__imlib_FileExists(const char *s)
{
   struct stat         st;

   return __imlib_FileStat(s, &st) == 0;
}

int
__imlib_FileIsFile(const char *s)
{
   struct stat         st;

   if (__imlib_FileStat(s, &st))
      return 0;

   return (S_ISREG(st.st_mode)) ? 1 : 0;
}

int
__imlib_FileIsDir(const char *s)
{
   struct stat         st;

   if (__imlib_FileStat(s, &st))
      return 0;

   return (S_ISDIR(st.st_mode)) ? 1 : 0;
}

time_t
__imlib_FileModDate(const char *s)
{
   struct stat         st;

   if (__imlib_FileStat(s, &st))
      return 0;

   return (st.st_mtime > st.st_ctime) ? st.st_mtime : st.st_ctime;
}

time_t
__imlib_FileModDateFd(int fd)
{
   struct stat         st;

   if (fstat(fd, &st) < 0)
      return 0;

   return (st.st_mtime > st.st_ctime) ? st.st_mtime : st.st_ctime;
}

int
__imlib_FileCanRead(const char *s)
{
   struct stat         st;

   if (__imlib_FileStat(s, &st))
      return 0;

   if (!(st.st_mode & (S_IRUSR | S_IRGRP | S_IROTH)))
      return 0;

   return access(s, R_OK) == 0 ? 1 : 0; // ??? TBD
}

char              **
__imlib_FileDir(const char *dir, int *num)
{
   int                 i, dirlen;
   int                 done = 0;
   DIR                *dirp;
   char              **names;
   struct dirent      *dp;

   if ((!dir) || (!*dir))
      return 0;
   dirp = opendir(dir);
   if (!dirp)
     {
        *num = 0;
        return NULL;
     }
   /* count # of entries in dir (worst case) */
   for (dirlen = 0; readdir(dirp) != NULL; dirlen++)
      ;
   if (!dirlen)
     {
        closedir(dirp);
        *num = dirlen;
        return NULL;
     }
   names = (char **)malloc(dirlen * sizeof(char *));

   if (!names)
      return NULL;

   rewinddir(dirp);
   for (i = 0; i < dirlen;)
     {
        dp = readdir(dirp);
        if (!dp)
           break;
        if ((strcmp(dp->d_name, ".")) && (strcmp(dp->d_name, "..")))
          {
             names[i] = strdup(dp->d_name);
             i++;
          }
     }

   if (i < dirlen)
      dirlen = i;               /* dir got shorter... */
   closedir(dirp);
   *num = dirlen;
   /* do a simple bubble sort here to alphanumberic it */
   while (!done)
     {
        done = 1;
        for (i = 0; i < dirlen - 1; i++)
          {
             if (strcmp(names[i], names[i + 1]) > 0)
               {
                  char               *tmp;

                  tmp = names[i];
                  names[i] = names[i + 1];
                  names[i + 1] = tmp;
                  done = 0;
               }
          }
     }
   return names;
}

void
__imlib_FileFreeDirList(char **l, int num)
{
   if (!l)
      return;
   while (num--)
      if (l[num])
         free(l[num]);
   free(l);
}

void
__imlib_FileDel(const char *s)
{
   if ((!s) || (!*s))
      return;
   unlink(s);
}

char               *
__imlib_FileHomeDir(int uid)
{
   static int          usr_uid = -1;
   static char        *usr_s = NULL;
   char               *s;
   struct passwd      *pwd;

   s = getenv("HOME");
   if (s)
      return strdup(s);

   if (usr_uid < 0)
      usr_uid = getuid();

   if ((uid == usr_uid) && (usr_s))
     {
        return strdup(usr_s);
     }

   pwd = getpwuid(uid);
   if (pwd)
     {
        s = strdup(pwd->pw_dir);
        if (uid == usr_uid)
           usr_s = strdup(s);
        return s;
     }

   return NULL;
}

int
__imlib_ItemInList(char **list, int size, char *item)
{
   int                 i;

   if (!list)
      return 0;
   if (!item)
      return 0;

   for (i = 0; i < size; i++)
     {
        if (!strcmp(list[i], item))
           return 1;
     }
   return 0;
}
