+++
title = 'Quick and easy offline backups with Duplicati'
date = 2024-09-27T16:31:40+02:00
type = "post"
draft = false
+++

Recently, I started using [Duplicati](https://duplicati.com/download) as a tool to create local, differential file backups to an external drive. My backup requirements are as follows:

* Backup to local drive, to be independent of availability and integrity of any cloud backup provider
* Differential backups, to avoid blowing up the storage size on the backup drive over time.
* Easy setup, without the need for a lot of configuration. I don't much enjoy administrative tasks like this and would prefer a solution that "just works".
* Cross-platform functionality. I often switch between computers running Windows, Linux and MacOS, and while I plan to create the backups from my Desktop PC running Windows, it might be necessary in the future to restore a backup to some other platform.
* Support for encryption is not required at the moment, as the volume I want to backup is already encrypted with [Cryptomator](https://cryptomator.org). Still, it's nice to have in case I decide in the future to backup other files which are not encrypted already.

Essentially, what I was looking for was a cross-platform variant of Apple's [Time Machine](https://support.apple.com/en-us/104984).

Fortunately for me, Duplicati seems to offer everything I need. Upon startup, it presents a clean graphical interface in the browser, which with a few clicks allowed me to point to my external drive as the backup destination and the directory containing my sensitive data as the source. AES-256 encryption can be optionally enabled.

Now, on the end of each work day, I connect the external drive to my Desktop and click a single button in the Duplicati GUI to backup a current snapshot of my data drive. Automatic timed updates are also possible, but for me the manual approach is enough, as I feel safer having the backup drive disconnected while it is not needed.

In case you want to backup to a remote location instead, this is also possible. The tool allows selecting amongst several protocols to do so. You can also define a custom deletion strategy for old backups, with a nice default of reducing the frequency of backups the older they become.

Restoring backups also works well via the integrated GUI. It allows you to select individual files and even search specific filenames in the backup. I did not yet try to restore files to a different device than the one that created the backup, but this should also be possible according to the documentation.

Duplicati also offers a command line interface with all the functionalities also available in the GUI. I had no reason to try this out though.

In all honesty, I really like Duplicati. It exactly matches my requirements for a backup solution and the whole setup process took maybe 10 minutes. In my opinion, this is a great example of a tool that achieves exactly what it sets out to do and provides sane defaults that make it immediately useful for a majority of users. The client is also open sourced under the MIT license, so I don't need to worry about it becoming unavailable in the future.
