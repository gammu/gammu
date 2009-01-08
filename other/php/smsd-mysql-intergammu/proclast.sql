-- phpMyAdmin SQL Dump
-- version 2.6.0-pl1
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Nov 14, 2004 at 06:03 AM
-- Server version: 4.0.20
-- PHP Version: 4.3.8
-- 
-- Database: `sms`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `proclast`
-- 

CREATE TABLE `proclast` (
  `lastinbox` int(11) unsigned NOT NULL default '0',
  KEY `lastinbox` (`lastinbox`)
) TYPE=MyISAM;

-- 
-- Dumping data for table `proclast`
-- 

INSERT INTO `proclast` VALUES (1);
