CREATE TABLE `users` (
  `id` int(11) NOT NULL,
  `email` varchar(64) NOT NULL,
  `name` varchar(64) NOT NULL,
  `password` varchar(64) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci

CREATE TABLE `friends` (
  `id` int(11) NOT NULL,
  `userid` int(11) DEFAULT NULL,
  `friendid` int(11) DEFAULT NULL,
  `block` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_friends_userid` (`userid`),
  KEY `fk_friends_friendid` (`friendid`),
  CONSTRAINT `fk_friends_friendid` FOREIGN KEY (`friendid`) REFERENCES `users` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_friends_userid` FOREIGN KEY (`userid`) REFERENCES `users` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci 

CREATE TABLE `friendapplys` (
  `id` int(11) NOT NULL,
  `fromid` int(11) DEFAULT NULL,
  `targetid` int(11) DEFAULT NULL,
  `status` varchar(16) DEFAULT 'Pending',
  PRIMARY KEY (`id`),
  KEY `fk_friendapplys_fromid` (`fromid`),
  KEY `fk_friendapplys_targetid` (`targetid`),
  CONSTRAINT `fk_friendapplys_fromid` FOREIGN KEY (`fromid`) REFERENCES `users` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_friendapplys_targetid` FOREIGN KEY (`targetid`) REFERENCES `users` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci

CREATE TABLE `groups` (
  `id` int(11) NOT NULL,
  `name` varchar(64) NOT NULL,
  `owner` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`),
  KEY `fk_groups_owner` (`owner`),
  CONSTRAINT `fk_groups_owner` FOREIGN KEY (`owner`) REFERENCES `users` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci

CREATE TABLE `groupusers` (
  `id` int(11) NOT NULL,
  `groupid` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `role` varchar(16) DEFAULT 'Member',
  `mute` tinyint(1) DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `fk_groupusers_groupid` (`groupid`),
  KEY `fk_groupusers_userid` (`userid`),
  CONSTRAINT `fk_groupusers_groupid` FOREIGN KEY (`groupid`) REFERENCES `groups` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_groupusers_userid` FOREIGN KEY (`userid`) REFERENCES `users` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci

CREATE TABLE `groupapplys` (
  `id` int(11) NOT NULL,
  `groupid` int(11) NOT NULL,
  `userid` int(11) NOT NULL,
  `status` varchar(16) DEFAULT 'Pending',
  PRIMARY KEY (`id`),
  KEY `fk_groupapplys_groupid` (`groupid`),
  KEY `fk_groupapplys_userid` (`userid`),
  CONSTRAINT `fk_groupapplys_groupid` FOREIGN KEY (`groupid`) REFERENCES `groups` (`id`) ON DELETE CASCADE,
  CONSTRAINT `fk_groupapplys_userid` FOREIGN KEY (`userid`) REFERENCES `users` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci

CREATE TABLE `files` (
  `filehash` varchar(128) NOT NULL,
  `filename` varchar(255) NOT NULL,
  `filesize` bigint(20) NOT NULL,
  `timestamp` timestamp NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`filehash`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci

CREATE TABLE `messages` (
  `id` int(11) NOT NULL,
  `senderid` int(11) DEFAULT NULL,
  `receiverid` int(11) DEFAULT NULL,
  `content` text DEFAULT NULL,
  `type` varchar(16) DEFAULT NULL,
  `status` varchar(16) DEFAULT NULL,
  `timestamp` timestamp(6) NULL DEFAULT current_timestamp(6),
  `isfile` tinyint(1) DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci