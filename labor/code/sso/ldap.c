#include <stdio.h>
#include <stdlib.h>
#include <ldap.h>
#include <errno.h>
#include <sys/time.h>

#define HANDLE_LDAP_ERROR(error, desc) \
	if (error != LDAP_SUCCESS) { \
		fprintf(stderr, "Error: %s: %s\n", \
			desc, ldap_err2string(error)); \
		ldap_perror(ld, desc); \
		exit(EXIT_FAILURE); \
	} 

int sasl_interact(struct ldap *ld, unsigned flags, void *defaults, void *interact);

void print_attribute_values(LDAP *ld, LDAPMessage *entry, const char *attribute);
void print_attributes(LDAP *ld, LDAPMessage *entry);
void print_entries(LDAP *ld, LDAPMessage *res);


void print_passwd(LDAP *ld, LDAPMessage *res);

void eval_result(LDAP *ld, LDAPMessage *res);
void print_group(LDAP *ld, LDAPMessage *entry);
void print_group_members(LDAP *ld, struct berval **members, const char *gidNumber);

int main(int argc, char **argv) {
	LDAP *ld;
	LDAPMessage *msg;
	int error;
	struct timeval timeout;
	timeout.tv_sec = 3;

	if (argc < 2) {
		fprintf(stderr, "Error: search filter expected!\n");
		return 1;
	}

	const char base[] = "ou=nsmlab,dc=lab,dc=nsmlab,dc=et,dc=hs-wismar,dc=de";
	const char uri[]  = "ldaps://ostseestadion.lab.nsmlab.et.hs-wismar.de";

	//const char base[] = "dc=franzke,dc=lan";
	//const char uri[] = "ldaps://home.franzke.lan";

	const char sasl_secprops[] = "maxssf=0";

	error = ldap_initialize(&ld, uri);
	HANDLE_LDAP_ERROR(error, "ldap_initialize");

	int protocol = LDAP_VERSION3;
	error = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol);
	HANDLE_LDAP_ERROR(error, "ldap_set_option PROTOCOL VERSION")

	error = ldap_set_option(ld, LDAP_OPT_X_SASL_SECPROPS, sasl_secprops);
	HANDLE_LDAP_ERROR(error, "ldap_set_option SASL_SECPROPS")
	int sizelimit;
	//ldap_get_option(ld, LDAP_SIZELIMIT_EXCEEDED, &sizelimit);
	sizelimit = LDAP_NO_LIMIT;

	ldap_set_option(ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF);
	

	error = ldap_sasl_interactive_bind_s(ld, "", "GSSAPI", NULL, NULL,
			LDAP_SASL_QUIET, sasl_interact, NULL);
	HANDLE_LDAP_ERROR(error, "ldap_sasl_interactive_bind_s")


	error = ldap_search_ext_s(ld, base, LDAP_SCOPE_SUB, argv[1] /* searchbase */,
			NULL /* attrs */, 0 /* attrsonly */,
			NULL /* serverctrls */, NULL /* clientctrls */,
			&timeout, sizelimit, &msg);

	error = ldap_result2error(ld, msg, 0);
	HANDLE_LDAP_ERROR(error, "ldap_search_ext_s")


	eval_result(ld, msg);
//	print_entries(ld, msg);
	

	ldap_msgfree(msg);
	ldap_unbind_ext_s(ld, NULL, NULL);
	return 0;
}

void eval_result(LDAP *ld, LDAPMessage *res) {
	LDAPMessage *entry;
	struct berval **object_class;
	int i, num;

	for (entry = ldap_first_entry(ld, res); entry != NULL;
			entry = ldap_next_entry(ld, entry)) {
		object_class = ldap_get_values_len(ld, entry, "objectclass");
		num = ldap_count_values_len(object_class);
		for (i = 0; i < num; ++i) {
			if (strcmp("user", object_class[i]->bv_val) == 0)
				print_passwd(ld, entry);
			if (strcmp("group", object_class[i]->bv_val) == 0)
				print_group(ld, entry);
		}
		ldap_value_free_len(object_class);

	}
	ldap_msgfree(entry);
}

void print_group_members(LDAP *ld, struct berval **members, const char *gidNumber) {
	LDAPMessage *msg;
	LDAPMessage *entry;
	struct berval **name;
	struct timeval timeout;
	timeout.tv_sec = 1;
	char *attrs[] = { "name", "gidNumber", NULL };
	int num = ldap_count_values_len(members);
	const char *filter_template = "(&(objectClass=user)(!(gidNumber=%s)))";
	int i,j;
	j = 0;
	for (i = 0; i < num; ++i) {
		size_t filter_size = strlen(filter_template)+strlen(gidNumber)-2+1;
		char *filter = malloc(filter_size);
		snprintf(filter, filter_size, filter_template, gidNumber);
		int error = ldap_search_ext_s(ld, members[i]->bv_val, LDAP_SCOPE_BASE, filter,
				attrs, 0, NULL, NULL, &timeout, 0, &msg);
		free(filter);
		HANDLE_LDAP_ERROR(error, "ldap_seach_ext_s member")
		entry = ldap_first_entry(ld, msg);
		if (entry == NULL)
			continue;
		name = ldap_get_values_len(ld, entry, "name");
		if (ldap_count_values_len(name)) {
			if (j++ > 0)
				putchar(',');
			fputs(name[0]->bv_val, stdout);
		}
		ldap_value_free_len(name);
		ldap_msgfree(msg);
	}
}

void print_group(LDAP *ld, LDAPMessage *entry) {
	struct berval **values;
	char *gidNumber = NULL;
	char *group_attrs[] = {"cn", "gidNumber", "member", NULL};
	int needed[] = {0, 1};

	int j;
	for (j = 0; j < 2; ++j) {
		values = ldap_get_values_len(ld, entry, group_attrs[needed[j]]);
		int num = ldap_count_values_len(values);
		ldap_value_free_len(values);
		if (num == 0)
			return;
	}

	int i = -1;
	while (group_attrs[++i] != NULL) {
		values = ldap_get_values_len(ld, entry, group_attrs[i]);
		if (i > 0)
			putchar(':');
		if (ldap_count_values_len(values)) {
			if (strcmp("member", group_attrs[i]) == 0)
				print_group_members(ld, values, gidNumber);
			else {
				if (strcmp("gidNumber", group_attrs[i]) == 0)
					gidNumber = strdup(values[0]->bv_val);
				fputs(values[0]->bv_val, stdout);
			}
		}
		ldap_value_free_len(values);
	}
	free(gidNumber);
	putchar('\n');
}

void print_passwd(LDAP *ld, LDAPMessage *entry) {
	struct berval **values;
	char *passwd_attrs[] = {"name", "uidNumber", "gidNumber", "displayName", "unixHomeDirectory", "loginShell", NULL};
	int needed[] = {0, 1, 2, 4, 5};
	int needed_size = 5;

	int j;
	for (j = 0; j < needed_size; ++j) {
		values = ldap_get_values_len(ld, entry, passwd_attrs[needed[j]]);
		int num = ldap_count_values_len(values);
		ldap_value_free_len(values);
		if (num == 0)
			return;
	}

	int i = -1;
	while (passwd_attrs[++i] != NULL) {
		values = ldap_get_values_len(ld, entry, passwd_attrs[i]);
		if (i > 0)
			putchar(':');
		if (ldap_count_values_len(values))
			fputs(values[0]->bv_val, stdout);
		ldap_value_free_len(values);
	}
	putchar('\n');
}

int sasl_interact(struct ldap *ld, unsigned flags, void *defaults, void *interact) {
	return LDAP_SUCCESS;
}
void print_attribute_values(LDAP *ld, LDAPMessage *entry, const char *attribute) {
	struct berval **values = ldap_get_values_len(ld, entry, attribute);
	int num = ldap_count_values_len(values);
	int i;
	for (i = 0; i<num; ++i)
		printf("%s: %s\n", attribute, values[i]->bv_val);
	ldap_value_free_len(values);
}

void print_attributes(LDAP *ld, LDAPMessage *entry) {
	BerElement *ber;
	char *attribute = ldap_first_attribute(ld, entry, &ber);
	do {
		print_attribute_values(ld, entry, attribute);
		ldap_memfree(attribute);
	} while ((attribute = ldap_next_attribute(ld, entry, ber)) != NULL);
	ber_free(ber, 0);
}

void print_entries(LDAP *ld, LDAPMessage *res) {
	LDAPMessage *entry;
	for (entry = ldap_first_entry(ld, res); entry != NULL;
			entry = ldap_next_entry(ld, entry)) {
		printf("dn: %s\n", ldap_get_dn(ld, entry));
		print_attributes(ld, entry);
	}
	ldap_msgfree(entry);
}
