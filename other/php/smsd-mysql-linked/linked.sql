-- phpMyAdmin SQL Dump
-- version 2.6.0-beta2
-- http://www.phpmyadmin.net
-- 
-- Host: localhost
-- Generation Time: Nov 05, 2004 at 11:26 PM
-- Server version: 4.0.18
-- PHP Version: 4.3.8
-- 
-- Database: `newsms`
-- 

-- --------------------------------------------------------

-- 
-- Table structure for table `linked`
-- 

CREATE TABLE `linked` (
  `ID` text NOT NULL,
  `Text_Category` text NOT NULL,
  `Text_Offer` text NOT NULL,
  `Text_Other` text NOT NULL,
  `PhoneNumber` text NOT NULL,
  `DateTime` timestamp(14) NOT NULL
) TYPE=MyISAM;

-- 
-- Dumping data for table `linked`
-- 

