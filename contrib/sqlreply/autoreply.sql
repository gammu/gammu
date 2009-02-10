-- phpMyAdmin SQL Dump
-- version 2.6.0-pl3
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Apr 14, 2005 at 08:04 PM
-- Server version: 4.1.8
-- PHP Version: 5.0.3
-- 
-- Database: `autoreply`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `actions`
-- 

CREATE TABLE `actions` (
  `ID` text NOT NULL,
  `ActionID` int(11) NOT NULL auto_increment,
  `User` text NOT NULL,
  `User2` text NOT NULL,
  `User3` text NOT NULL,
  `Pass` text NOT NULL,
  `Pass2` text NOT NULL,
  `Pass3` text NOT NULL,
  `DB` text NOT NULL,
  `DB2` text NOT NULL,
  `DB3` text NOT NULL,
  `PC` text NOT NULL,
  `PC2` text NOT NULL,
  `PC3` text NOT NULL,
  `SQL` text NOT NULL,
  KEY `ActionID` (`ActionID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=3 ;

-- 
-- Dumping data for table `actions`
-- 

INSERT INTO `actions` VALUES ('0', 1, 'root', 'root', 'root', '', '', '', 'autoreplytest', 'autoreplytest', 'autoreplytest', 'localhost', 'localhost', 'localhost', 'select * from a where txt=''wartosc''\r\n{\r\n  daemon_run_program <c:\\windows\\notepad.exe<d:\\ala.txt<\r\n');

-- --------------------------------------------------------

-- 
-- Table structure for table `rules`
-- 

CREATE TABLE `rules` (
  `ID` text NOT NULL,
  `RuleID` int(11) NOT NULL auto_increment,
  `DB` text NOT NULL,
  `SQL` text NOT NULL,
  `User` text NOT NULL,
  `Pass` text NOT NULL,
  `PC` text NOT NULL,
  UNIQUE KEY `RuleID` (`RuleID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 AUTO_INCREMENT=2 ;

-- 
-- Dumping data for table `rules`
-- 

INSERT INTO `rules` VALUES ('0', 1, 'autoreplytest', 'select * from a where txt=''wartosc''', 'root', '', 'localhost');
