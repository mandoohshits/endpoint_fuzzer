import socket
import whois
import dns.resolver

def get_ip_info(domain):
    print("\n[+] --- IP & REVERSE DNS ---")
    try:
        ip_address = socket.gethostbyname(domain)
        print(f"Target Domain : {domain}")
        print(f"IP Address    : {ip_address}")
        
        try:
            host_name = socket.gethostbyaddr(ip_address)[0]
            print(f"Reverse Host  : {host_name}")
        except socket.herror:
            print("Reverse Host  : Could not resolve PTR record")
            
    except socket.gaierror:
        print("[-] Could not resolve domain to IP address.")

def get_whois_info(domain):
    print("\n[+] --- WHOIS DOMAIN DATA ---")
    try:
        w = whois.whois(domain)
        
        # Registrar and Creation info
        registrar = w.registrar if w.registrar else "Unknown"
        creation_date = w.creation_date
        expiration_date = w.expiration_date
        
        # Handle cases where WHOIS returns lists for dates
        if isinstance(creation_date, list):
            creation_date = creation_date[0]
        if isinstance(expiration_date, list):
            expiration_date = expiration_date[0]

        print(f"Registrar     : {registrar}")
        print(f"Created On    : {creation_date}")
        print(f"Expires On    : {expiration_date}")
        
        if w.name_servers:
            ns_list = w.name_servers if isinstance(w.name_servers, list) else [w.name_servers]
            print(f"Name Servers  : {', '.join(ns_list[:3])}")
            
    except Exception as e:
        print(f"[-] WHOIS lookup failed: {e}")

def get_dns_records(domain):
    print("\n[+] --- DNS RECORDS ---")
    record_types = ['A', 'MX', 'TXT']
    
    for rtype in record_types:
        try:
            answers = dns.resolver.resolve(domain, rtype)
            print(f"\n[{rtype} Records]")
            for rdata in answers:
                print(f"  -> {rdata.to_text()}")
        except Exception:
            print(f"\n[{rtype} Records]\n  -> None found or lookup failed.")

def run_recon(target):
    print("==================================================")
    print(f"         AUTOMATED OSINT RECON TOOL              ")
    print("==================================================")
    
    # Strip protocol prefix if user enters http(s)://
    target = target.replace("https://", "").replace("http://", "").strip("/")
    
    get_ip_info(target)
    get_whois_info(target)
    get_dns_records(target)
    
    print("\n==================================================")

if __name__ == "__main__":
    # Test on a public domain like scanme.nmap.org or github.com
    target_domain = "github.com" 
    run_recon(target_domain)