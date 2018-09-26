# Alibaba feiyan cloud:  
	This is a gateway demo which connects to alibaba feiyan cloud and runs on SOC mtk7620.  
	  
	This Demo Bridages Messages Between NXP and Alibaba Cloud .  

# Author:   
	dyx/tech32@hzdusun.com  
	au/ghaucience@gmail.com

# Use:   
	1.  
	2. make  

# !!!:  
	...  

# Developing:  
	1. product_sub & product module   [ok ]  
	3. ubus proto -> afly module [ok ]
	3. device reg->create->login->unlogin->destroy->unreg [ok ]
	4. dev sevice framework  add pass/del pass/storage pass  [ok]
	5. report [ok]
	6. sdk watch
	7. add product & del product link debug [ok ]  
	8. 网关更新nxp程序,基线和旧版本不一样
	9. json及内存泄露问题
	10. 增加回连
	11. report 根据类型查找 
	12. 属性跟新的时候才上报或者才保存
	13. cmd.c 当输入为是前缀空格的时候的 bug [ok]
	14. 增加subdev的时候出发list上报 [ok]

# Version:  
	V1.0.0_afly_beta0:  
		- Functions:  
			*  
		- Changes:  
			*  
		- Problem:  
			*  
		- Todo:  
			*  

# Lays 
	
					-AFly-
	---------------------------------------
	-Z3- -nxp-   -storage-
	-----------
	 -uproto-

