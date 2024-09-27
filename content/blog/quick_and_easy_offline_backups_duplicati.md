+++
title = 'Quick and easy offline backups with Duplicati'
date = 2024-09-27T16:31:40+02:00
type = "post"
draft = true
+++

## Introduction

Recently, I was looking for a solution for secure backups to a local drive. I already keep all important data inside a locally encrypted [Cryptomator Vault](https://cryptomator.org), which I store with a cloud storage provider, so it is synced among my 2-3 different connected devices. With this, I'm already reasonably well guarded against any or all of my local hard drives failing and corrupting or losing data. In addition, I don't have to worry about someone accessing my data by attacking the cloud storage provider, as I store just an encrypted vault online, for which only I have the decryption key.

After asking around among my peer group and looking online a bit, this seems to be the way to go for most, even technically inclined, people today (sans the local encryption in most cases), due to the ease of setting up Dropbox/Google Drive/etc. and the immediate availability of files across devices.

However, while cloud storage is definitely convenient, of course there is no guarantee that files won't be corrupted on the remote storage, either on accident or due to a deliberate attack. Just as an example, a quick Google search yielded [this Hacker News thread from 2014](https://news.ycombinator.com/item?id=8440985), where apparently Dropbox introduced a bug which caused data loss for some users. Whenever something like this happens, due to the bi-directional sync functionality of most cloud storage services, there is the danger of a corrupted file being synced back to all connected devices, deleting any intact copy of the original data!

Though unlikely, this scenario had been worrying me for quite some time now, so I finally decided to do something about it by implementing a backup strategy for my sensitive data. I opted for a backup to a local drive, to be independent of the availability and integrity of any 3rd party provider or my network connection. As a backup medium, I bought a reasonably priced, well reviewed USB hard drive (I don't want to advertise any specific supplier here, IMHO any of the generally known brands should be ok).

In addition, my requirements were as follows:

* Differential backups, to avoid blowing up the storage size on the backup drive over time.
* Easy setup, without the need for a lot of configuration. I don't much enjoy administrative tasks like this and would prefer a solution that "just works".
* Cross-platform functionality. I often switch between computers running Windows, Linux and MacOS, and while I plan to create the backups from my Desktop PC running Windows, it might be necessary in the future to restore a backup to some other platform.
* Support for encryption is not required at the moment, as the vault I want to backup is already encrypted. Still, it's nice to have in case I decide in the future to backup other files which are not part of the Cryptomator Vault.

Essentially, what I was looking for was a cross-platform variant of Apple's [Time Machine](https://support.apple.com/en-us/104984).

## Duplicati

Fortunately for me, [Duplicati](https://duplicati.com/download) seems to offer everything I need. Upon startup, it presents a clean graphical interface in the browser, which with a few clicks allowed me to point to my external drive as the backup destination and the directory containing my sensitive data as the source. AES-256 encryption can be optionally enabled.

Now, on the end of each work day, I connect the external drive to my Desktop and click a single button in the Duplicati GUI to backup a current snapshot of my data drive. Automatic timed updates are also possible, but for me the manual approach is enough, as I feel safer having the backup drive disconnected while it is not needed.

In case you want to backup to a remote location instead, this is also possible. The tool allows selecting amongst several protocols to do so. You can also define a custom deletion strategy for old backups, with a nice default of reducing the frequency of backups the older they become.

Restoring backups also works well via the integrated GUI. It allows you to select individual files and even search specific filenames in the backup. I did not yet try to restore files to a different device than the one that created the backup, but this should also be possible according to the documentation.

Duplicati also offers a command line interface with all the functionalities also available in the GUI. I had no reason to try this out though.

## Verdict

I really like Duplicati. It exactly matches my requirements for a backup solution and the whole setup process took maybe 10 minutes. In my opinion, this is a great example of a tool that manages to do exactly what it sets out to do and providing sane defaults that make it immediately useful for a majority of users. The client is also open sourced under the MIT license, so I don't need to worry about it becoming unavailable in the future.